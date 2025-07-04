#pragma once

class GameCharacter;

/// <summary>
/// コマンドパターンの基底インターフェース
/// プレイヤーと敵AIが同じ行動を実行できるようにする
/// </summary>
class ICommand {
public:
    virtual ~ICommand() = default;
    
    /// <summary>
    /// コマンドを実行する
    /// </summary>
    /// <param name="character">実行対象のキャラクター</param>
    /// <param name="deltaTime">フレーム時間</param>
    virtual void Execute(GameCharacter* character, float deltaTime) = 0;
    
    /// <summary>
    /// コマンドが完了したかどうか
    /// </summary>
    /// <returns>完了していればtrue</returns>
    virtual bool IsComplete() const = 0;
    
    /// <summary>
    /// コマンドをリセットして再利用可能にする
    /// </summary>
    virtual void Reset() = 0;
};