# Beeep Message Bus Unreal Engine

This plugin is inspired by
the [GameplayMessageRouter](https://github.com/EpicGames/UnrealEngine/tree/release/Samples/Games/Lyra/Plugins/GameplayMessageRouter)
in the Lyra project, with some enhanced functionality.

In the original `GameplayMessageRouter`, you cannot synchronously unregister a listener while receiving a message
because it would result in removing elements during array iteration.

This plugin solves this issue by implementing a `MarkDead` approach.

To avoid writing custom `K2_Node` code, this plugin uses `Instancedstruct` as the type for the payload, and implemented
dynamic types through InstancedStruct while maintaining polymorphism.

### Example

![img.png](img.png)

You can also register listener like below
![img_1.png](img_1.png)