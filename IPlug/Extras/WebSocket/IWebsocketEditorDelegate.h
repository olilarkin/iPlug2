#pragma once

#include "IGraphicsEditorDelegate.h"
#include "IWebsocketServer.h"
#include "IPlugStructs.h"
#include "IPlugQueue.h"

/**
 * @file
 * @copydoc IWebsocketEditorDelegate
 */

/** An IEditorDelegate base class that embeds a websocket server.
 *  WARNING: this is an extremely flaky, proof of concept feature
 */
class IWebsocketEditorDelegate : public IGEditorDelegate, public IWebsocketServer
{
public:
  IWebsocketEditorDelegate(int nParams);
  virtual ~IWebsocketEditorDelegate();
 
  //IWebsocketServer
  //THESE MESSAGES ARE ALL CALLED ON SERVER THREADS - 1 PER WEBSOCKET CONNECTION
  void OnWebsocketReady(int idx) override;
  void OnWebsocketDisconnected(int idx) override;
  bool OnWebsocketText(int idx, const char* pStr, size_t dataSize) override;
  bool OnWebsocketData(int idx, void* pData, size_t dataSize) override;

  //IEditorDelegate
  void SendMidiMsgFromUI(const IMidiMsg& msg) override;
  void SendSysexMsgFromUI(const ISysEx& msg) override;
  void SendArbitraryMsgFromUI(int messageTag, int controlTag, int dataSize, const void* pData) override;
//void BeginInformHostOfParamChangeFromUI(int paramIdx) override;
  void SendParameterValueFromUI(int paramIdx, double normalizedValue) override;
//void EndInformHostOfParamChangeFromUI(int paramIdx) override;

  void SendControlValueFromDelegate(int controlTag, double normalizedValue) override;
  void SendControlMsgFromDelegate(int controlTag, int messageTag, int dataSize, const void* pData) override;
  void SendArbitraryMsgFromDelegate(int messageTag, int dataSize, const void* pData) override;
  void SendMidiMsgFromDelegate(const IMidiMsg& msg) override;
  void SendSysexMsgFromDelegate(const ISysEx& msg) override;
  
  // Call this repeatedly from OnIdle() in order to handle incoming data
  void ProcessWebsocketQueue();
  
private:
  void DoSPVFDToClients(int paramIdx, double value, int connectionIdx, int excludeIdx);

  struct ConnectionQueueHolder
  {
    IPlugQueue<ParamTuple> ParamChangesFromClient {PARAM_TRANSFER_SIZE};
    IPlugQueue<IMidiMsg> MIDIFromClient {MIDI_TRANSFER_SIZE};
    int connectionIdx;
    
    ConnectionQueueHolder(int idx)
    : connectionIdx(idx)
    {};
  };
  
  std::vector<std::unique_ptr<ConnectionQueueHolder>> mConnectionQueues; // SPSC queues for each connection
  std::atomic<bool> mNewConnection {false}; // signals a new connection to synchronize all clients with server
};
