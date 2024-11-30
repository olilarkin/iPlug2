import RealityKit
import SwiftUI

struct ContentView: View {
    @State private var rotationAngles: SIMD3<Float> = [0, 0, 0]
    @State private var modulationTimer: Timer?
    @State private var time: Double = 0.0
    @State private var lastRotationUpdateTime = CACurrentMediaTime()
    @EnvironmentObject var state: IPlugRealityViewState

    
    let overallSphereRadius: Float = 0.5
    let rootEntity: Entity = Entity()
    
    var body: some View {
        RealityView { content in
            // add outer spheres
            for sphere in getFibonacciLattice(radius: overallSphereRadius) {
                rootEntity.addChild(sphere)
                rootEntity.scale = SIMD3<Float>.init(x: 0.5, y: 0.5, z: 0.5)
            }
            
            // add overall sphere
            let mesh = MeshResource.generateSphere(radius: overallSphereRadius)
            var material = PhysicallyBasedMaterial()
            material.baseColor.tint = .blue
            material.blending = .transparent(opacity: 0.25)
            rootEntity.components.set(ModelComponent(mesh: mesh, materials: [material]))
            content.add(rootEntity)
        }
        .onAppear { startTimer() }
        .onDisappear { stopTimer() }
    }
    
    func startTimer() {
        modulationTimer = Timer.scheduledTimer(withTimeInterval: 1/120.0, repeats: true) { _ in
            let currentTime = CACurrentMediaTime()
            let frameDuration = currentTime - lastRotationUpdateTime
            
            // Rotate along each axis at a different rate for a wonky roll effect
            rotationAngles.x += Float(frameDuration * 0.25)
            rotationAngles.y += Float(frameDuration * 0.15)
            rotationAngles.z += Float(frameDuration * 0.1)
            
            let rotationX = simd_quatf(angle: rotationAngles.x, axis: [1, 0, 0])
            let rotationY = simd_quatf(angle: rotationAngles.y, axis: [0, 1, 0])
            let rotationZ = simd_quatf(angle: rotationAngles.z, axis: [0, 0, 1])
            rootEntity.transform.rotation = rotationX * rotationY * rotationZ
            
            // scale/breathing logic
            self.time += frameDuration
            let scale = Float(state.params[0].value)
            rootEntity.scale = SIMD3<Float>.init(x: scale, y: scale, z: scale)
            
            lastRotationUpdateTime = currentTime
        }
    }
    
    func getFibonacciLattice(numberOfSpheres: Int = 300,
                             radius: Float = 0.7) -> [Entity] {
        var spheres = [Entity]()
        for i in 0..<numberOfSpheres {
            let theta = acos(1 - 2 * Float(i + 1) / Float(numberOfSpheres + 1))
            let phi = Float(i) * .pi * (1 + sqrt(5)) // Golden angle in radians
            
            let x = radius * sin(theta) * cos(phi)
            let y = radius * sin(theta) * sin(phi)
            let z = radius * cos(theta)
            let position = SIMD3<Float>(x, y, z)
            
            let sphere = createSmallSphereEntity()
            sphere.position = position
            spheres.append(sphere)
        }
        return spheres
    }
    
    func createSmallSphereEntity() -> Entity {
        let mesh = MeshResource.generateSphere(radius: 0.2)
        let material = SimpleMaterial(color: .blue, isMetallic: false)
        let sphereEntity = ModelEntity(mesh: mesh, materials: [material])
        return sphereEntity
    }
    
    func stopTimer() {
        modulationTimer?.invalidate()
        modulationTimer = nil
    }
}

#Preview {
    ContentView()
}
