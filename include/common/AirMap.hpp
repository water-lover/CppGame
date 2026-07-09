#ifndef AIRMAP_HPP
#define AIRMAP_HPP

#include <vector>
#include "common/Actor.hpp"

// ── AirMap ──────────────────────────────────────────────────────────────────
/// 精灵集合容器 — View 遍历此集合绘制所有对象
/// 
/// 这是 View 唯一能看到的"数据窗口"。
/// ViewModel 每帧将内部数据同步到此容器，View 通过 const AirMap* 只读遍历。
class AirMap {
public:
    /// 清空所有精灵
    void clear();

    /// 当前精灵数量
    size_t size() const;

    /// 获取第 idx 个精灵（只读）
    const Actor& getAt(size_t idx) const;

    /// 获取第 idx 个精灵（可写，供 ViewModel 同步数据）
    Actor& getAt(size_t idx);

    /// 追加一个精灵
    void append(const Actor& actor);

private:
    std::vector<Actor> m_actors;
};

#endif // AIRMAP_HPP
