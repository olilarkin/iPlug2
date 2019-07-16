#include "IWebsocketEditorDelegate.h"
#include "IPlugStructs.h"

IWebsocketEditorDelegate::IWebsocketEditorDelegate(int nParams)
: IGEditorDelegate(nParams)
{
}

IWebsocketEditorDelegate::~IWebsocketEditorDelegate()
{
}

void IWebsocketEditorDelegate::OnWebsocketReady(int connIdx)
{
  mConnectionQueues.push_back(std::make_unique<ConnectionQueueHolder>(connIdx)); //TODO: need a better way to identify client
}

void IWebsocketEditorDelegate::OnWebsocketDisconnected(int connIdx)
{
  mConnectionQueues.erase(std::find_if(mConnectionQueues.begin(), mConnectionQueues.end(), [connIdx] (const std::unique_ptr<ConnectionQueueHolder>& cqh)
                                                                                            { return cqh.get()->connectionIdx == connIdx; }));
  //TODO: need a better way to identify client
}

bool IWebsocketEditorDelegate::OnWebsocketText(int connIdx, const char* pStr, size_t dataSize)
{
  if(dataSize == 11)
  {
    if(strcmp(pStr, "syncRequest"))
      mNewConnection.store(true);
  }
  
  return true; // return true to keep the connection open
}

//In OnWebsocketData (which gets called on server connection thread)
//when the user of a client edits their remote UI, push the edit into the SPSC queue for this thread
//to be handled on the main thread in ProcessWebSocketQueue
//where it will be used to update the server side and other clients
bool IWebsocketEditorDelegate::OnWebsocketData(int connIdx, void* pData, size_t dataSize)
{
  uint8_t* pByteData = (uint8_t*) pData;
  int pos = 6;

  // Send Parameter Value from UI
  if (memcmp(pData, "SPVFUI" , 6) == 0)
  {
    int paramIdx = * ((int*)(pByteData + pos)); pos+= 4;
    double value = * ((double*)(pByteData + pos)); pos += 8;
    
    mConnectionQueues[connIdx]->ParamChangesFromClient.Push(ParamTuple { paramIdx, value } );
  }
  // Send MIDI Message from UI
  else if (memcmp(pData, "SMMFUI" , 6) == 0)
  {
    IMidiMsg msg;
    msg.mStatus = * ((uint8_t*)(pByteData + pos)); pos++;
    msg.mData1 = * ((uint8_t*)(pByteData + pos)); pos++;
    msg.mData2 = * ((uint8_t*)(pByteData + pos)); pos++;

     mConnectionQueues[connIdx]->MIDIFromClient.Push(msg);
  }
  // Send Sysex Message from UI
  else if (memcmp(pData, "SSMFUI" , 6) == 0)
  {
    //TODO: how are we going to queue
  }
  // Send Arbitary Message from UI
  else if (memcmp(pData, "SAMFUI" , 6) == 0) // send arbitrary message from user interface
  {
    //TODO:
  }

  return true;
}

void IWebsocketEditorDelegate::SendMidiMsgFromUI(const IMidiMsg& msg)
{
  IByteChunk data;
  data.PutStr("SMMFD");
  data.Put(&msg.mStatus);
  data.Put(&msg.mData1);
  data.Put(&msg.mData2);

  // Server side UI edit, send to clients
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendMidiMsgFromUI(msg);
}

void IWebsocketEditorDelegate::SendSysexMsgFromUI(const ISysEx& msg)
{
  IByteChunk data;
  data.PutStr("SSMFD");
  data.Put(&msg.mSize);
  data.PutBytes(&msg.mData, msg.mSize);
  
  // Server side UI edit, send to clients
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendSysexMsgFromUI(msg);
}

void IWebsocketEditorDelegate::SendArbitraryMsgFromUI(int messageTag, int controlTag, int dataSize, const void* pData)
{
  IByteChunk data;
  data.PutStr("SSMFD");
  data.Put(&messageTag);
  data.Put(&controlTag);
  data.Put(&dataSize);
  data.PutBytes(pData, dataSize);
  
  // Server side UI edit, send to clients
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendArbitraryMsgFromUI(messageTag, controlTag, dataSize, pData);
}

//void IWebsocketEditorDelegate::BeginInformHostOfParamChangeFromUI(int paramIdx)
//{
//  IGEditorDelegate::BeginInformHostOfParamChangeFromUI(paramIdx);
//}

void IWebsocketEditorDelegate::SendParameterValueFromUI(int paramIdx, double value)
{
  DoSPVFDToClients(paramIdx, value, -1, -1 /*Server-side UI edit, send to all clients*/);
  IGEditorDelegate::SendParameterValueFromUI(paramIdx, value);
}

//void IWebsocketEditorDelegate::EndInformHostOfParamChangeFromUI(int paramIdx)
//{
//  IGEditorDelegate::EndInformHostOfParamChangeFromUI(paramIdx);
//}

void IWebsocketEditorDelegate::SendControlValueFromDelegate(int controlTag, double normalizedValue)
{
  IByteChunk data;
  data.PutStr("SCVFD");
  data.Put(&controlTag);
  data.Put(&normalizedValue);
  
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendControlValueFromDelegate(controlTag, normalizedValue);
}

void IWebsocketEditorDelegate::SendControlMsgFromDelegate(int controlTag, int messageTag, int dataSize, const void* pData)
{
  IByteChunk data;
  data.PutStr("SCMFD");
  data.Put(&controlTag);
  data.Put(&messageTag);
  data.Put(&dataSize);
  data.PutBytes(pData, dataSize);
  
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendControlMsgFromDelegate(controlTag, messageTag, dataSize, pData);
}

void IWebsocketEditorDelegate::SendArbitraryMsgFromDelegate(int messageTag, int dataSize, const void* pData)
{
  IByteChunk data;
  data.PutStr("SAMFD");
  data.Put(&messageTag);
  data.Put(&dataSize);
  data.PutBytes(pData, dataSize);
  
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendArbitraryMsgFromDelegate(messageTag, dataSize, pData);
}

void IWebsocketEditorDelegate::SendMidiMsgFromDelegate(const IMidiMsg& msg)
{
  IByteChunk data;
  data.PutStr("SMMFD");
  data.Put(&msg.mStatus);
  data.Put(&msg.mData1);
  data.Put(&msg.mData2);

  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendMidiMsgFromDelegate(msg);
}

void IWebsocketEditorDelegate::SendSysexMsgFromDelegate(const ISysEx& msg)
{
  IByteChunk data;
  data.PutStr("SSMFD");
  data.Put(&msg.mSize);
  data.PutBytes(msg.mData, msg.mSize);
  
  SendDataToConnection(-1, data.GetData(), data.Size());
  
  IGEditorDelegate::SendSysexMsgFromDelegate(msg);
}

void IWebsocketEditorDelegate::ProcessWebsocketQueue()
{
  for (int el =0; el < mConnectionQueues.size(); el++)
  {
    ConnectionQueueHolder* pQueueHolder = mConnectionQueues[el].get();
    
    while(pQueueHolder->ParamChangesFromClient.ElementsAvailable())
    {
      ParamTuple p;
      pQueueHolder->ParamChangesFromClient.Pop(p);
      
      ENTER_PARAMS_MUTEX;
      IParam* pParam = GetParam(p.idx);
      
      if(pParam)
        pParam->SetNormalized(p.value);
      LEAVE_PARAMS_MUTEX;
      
      OnParamChange(p.idx, kHost, -1);
      OnParamChangeUI(p.idx, kHost);
      
      // echo parameter changes to other clients
      DoSPVFDToClients(p.idx, p.value, -1, pQueueHolder->connectionIdx /* exclude = connection */);
      
      SendParameterValueFromDelegate(p.idx, p.value, true); // TODO:  if the parameter hasn't changed maybe we shouldn't do anything?
    }
    
    while (pQueueHolder->MIDIFromClient.ElementsAvailable()) {
      IMidiMsg msg;
      pQueueHolder->MIDIFromClient.Pop(msg);
      IGEditorDelegate::SendMidiMsgFromDelegate(msg); // Call the superclass, since we don't want to send another MIDI message to the websocket
      DeferMidiMsg(msg); // can't just call SendMidiMsgFromUI here which would cause a feedback loop
    }
  }
  
  //TODO: really need to serialize state and send it to the clients
  if(mNewConnection.load())
  {
    for (int i=0; i< NParams(); i++) {
      DoSPVFDToClients(i, GetParam(i)->GetNormalized(), -1, -1);
    }

    mNewConnection.store(false);
  }
}

void IWebsocketEditorDelegate::DoSPVFDToClients(int paramIdx, double value, int connectionIdx, int excludeIdx)
{
  IByteChunk data;
  data.PutStr("SPVFD");
  data.Put(&paramIdx);
  data.Put(&value);
  SendDataToConnection(connectionIdx, data.GetData(), data.Size(), excludeIdx);
}
