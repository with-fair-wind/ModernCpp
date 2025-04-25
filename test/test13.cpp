#include <variant>
#include <unordered_map>
#include <iostream>
#include <string>
#include <optional>
#include <map>

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

enum class ViewBlockType
{
    Block1,
    Block2,
    Block3
};
enum class LogicBlockType
{
    SchematicLine,
    WiringNode
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

template <typename Mode, typename Type>
class IBlockRegistry
{
public:
    virtual ~IBlockRegistry() = default;

    virtual Mode getCurMode() const = 0; // 获取当前模式

    virtual int getA() const { return a; } // 返回自身引用
    virtual IBlockRegistry &setA(int a)
    {
        this->a = a;
        return *this;
    } // 设置自身引用

    double getB() const { return b; } // 返回自身引用
    IBlockRegistry &setB(double b)
    {
        this->b = b;
        return *this;
    } // 设置自身引用

    virtual void registerBlock(Mode mode, Type type, const Block &block)
    {
        blocks_[mode][type] = block;
    }

    virtual Block *getBlock(Mode mode, Type type)
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

protected:
    std::map<Mode, std::map<Type, Block>> blocks_; // 用于存储块数据
    int a;
    double b;
};

class ViewBlockRegistry : public IBlockRegistry<ViewMode, ViewBlockType>
{
public:
    // 子类不再需要实现 registerBlock 和 getBlock，基类已经处理了。
    // 只需要指定正确的 Mode 和 Type 类型
    virtual ViewMode getCurMode() const { return ViewMode::Top; }
};

class LogicBlockRegistry : public IBlockRegistry<LogicMode, LogicBlockType>
{
public:
    // 同上，继承基类实现逻辑
    virtual LogicMode getCurMode() const { return LogicMode::MainWiring; }
};

int main()
{
    ViewBlockRegistry viewReg;
    LogicBlockRegistry logicReg;

    viewReg.setA(10).setB(100.);
    ViewMode mode = viewReg.getCurMode();
    std::cout << static_cast<int>(mode) << " " << viewReg.getA() << " " << viewReg.getB() << std::endl;

    Block topBlock = {1, "Top", 1.0, true};
    Block schematicBlock = {2, "Schematic", 2.0, false};

    // 注册数据
    viewReg.registerBlock(ViewMode::Top, ViewBlockType::Block1, topBlock);
    logicReg.registerBlock(LogicMode::Schematic, LogicBlockType::SchematicLine, schematicBlock);

    // 查询数据
    if (auto blk = viewReg.getBlock(ViewMode::Top, ViewBlockType::Block1))
    {
        blk->print(); // ✅ 输出 TopBlock
    }

    if (auto blk = logicReg.getBlock(LogicMode::Schematic, LogicBlockType::SchematicLine))
    {
        blk->print(); // ✅ 输出 SchematicBlock
    }

    return 0;
}
