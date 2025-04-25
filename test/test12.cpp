#include <variant>
#include <unordered_map>
#include <iostream>
#include <string>
#include <optional>

enum class Category
{
    View,
    Logic
};
enum class ViewMode
{
    Top,
    Left,
    Front
};
enum class LogicMode
{
    Schematic,
    MainWiring
};

// 统一的 Block 数据结构
struct Block
{
    int id;
    std::string name;
    double param1;
    bool flag;

    void print() const
    {
        std::cout << "Block[id=" << id << ", name=" << name
                  << ", param1=" << param1 << ", flag=" << flag << "]\n";
    }
};

template <typename Mode>
class IBlockRegistry
{
public:
    virtual ~IBlockRegistry() = default;
    virtual void registerBlock(Mode mode, int type, const Block &block) = 0;
    virtual Block *getBlock(Mode mode, int type) = 0;
};

class ViewBlockRegistry : public IBlockRegistry<ViewMode>
{
public:
    void registerBlock(ViewMode mode, int type, const Block &block) override
    {
        blocks_[mode][type] = block;
    }

    Block *getBlock(ViewMode mode, int type) override
    {
        auto mit = blocks_.find(mode);
        if (mit != blocks_.end())
        {
            auto tit = mit->second.find(type);
            if (tit != mit->second.end())
                return &tit->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<ViewMode, std::unordered_map<int, Block>> blocks_;
};

class LogicBlockRegistry : public IBlockRegistry<LogicMode>
{
public:
    void registerBlock(LogicMode mode, int type, const Block &block) override
    {
        blocks_[mode][type] = block;
    }

    Block *getBlock(LogicMode mode, int type) override
    {
        auto mit = blocks_.find(mode);
        if (mit != blocks_.end())
        {
            auto tit = mit->second.find(type);
            if (tit != mit->second.end())
                return &tit->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<LogicMode, std::unordered_map<int, Block>> blocks_;
};

int main()
{
    ViewBlockRegistry viewReg;
    LogicBlockRegistry logicReg;

    Block topBlock = {1, "Top", 1.0, true};
    Block schematicBlock = {2, "Schematic", 2.0, false};

    viewReg.registerBlock(ViewMode::Top, 1, topBlock);
    logicReg.registerBlock(LogicMode::Schematic, 2, schematicBlock);

    if (auto blk = viewReg.getBlock(ViewMode::Top, 1))
    {
        blk->print();
    }
    if (auto blk = logicReg.getBlock(LogicMode::Schematic, 2))
    {
        blk->print();
    }

    return 0;
}
