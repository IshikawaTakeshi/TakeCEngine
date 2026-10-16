# NavigationAgentComponent

`AIControllerComponent`のMoveTo/Stop要求を`NavigationAgentComponent`が受け取り、
ウェイポイントを`CharacterMotorComponent`で追従します。攻撃命令は他のHandlerへ渡します。
`AIMovementComponent`は旧来の直線移動Handlerです。同じGameObjectへ両方は追加できません。

```cpp
auto& controller = enemy.AddComponent<TakeC::AIControllerComponent>();
auto& navigation = enemy.AddComponent<TakeC::NavigationAgentComponent>();
auto& motor = enemy.AddComponent<TakeC::CharacterMotorComponent>();
motor.SetSpeed(2.0f);
controller.GetCommandBuffer().RequestMoveTo({10.0f, 0.0f, 0.0f});
world.LateUpdate(deltaTime);
```

ワールドに`IPathfindingService`を登録すると、LateUpdateで同期的に経路を求めます。
サービスがない場合は目的地だけをウェイポイントとする直線移動です。
返す経路は空ではなく、最後の点を要求された目的地と一致させてください。
探索不能なら`std::nullopt`を返します。

```cpp
world.GetServices().Register<TakeC::IPathfindingService>(pathfindingService);
```

同じ目的地への繰り返しMoveToでは再探索しません。目的地が
`GetRepathDistance()`より大きく動いた場合や`RequestRepath()`を呼んだ場合に再探索します。
`NoPath`時は停止し、同じ目的地への再要求では探索を繰り返しません。
経路データ更新を検知して再探索する仕組みとA*の実装は次のステップです。
