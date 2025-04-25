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

struct TopBlock1
{
    int width;
    std::string label;
};
struct LeftBlock2
{
    double angle;
};
struct SchematicBlock1
{
    double voltage;
    bool isActive;
};
using BlockVariant = std::variant<
    TopBlock1,
    LeftBlock2,
    SchematicBlock1>;

template <typename ModeEnum>
using ModeMap = std::unordered_map<ModeEnum, std::unordered_map<int, BlockVariant>>;

class BlockRegistry
{
public:
    template <typename Mode>
    void registerBlock(Category category, Mode mode, int type, BlockVariant block)
    {
        if constexpr (std::is_same_v<Mode, ViewMode>)
        {
            if (category != Category::View)
                throw std::invalid_argument("Invalid Category for ViewMode");
            viewBlocks_[mode][type] = std::move(block);
        }
        else if constexpr (std::is_same_v<Mode, LogicMode>)
        {
            if (category != Category::Logic)
                throw std::invalid_argument("Invalid Category for LogicMode");
            logicBlocks_[mode][type] = std::move(block);
        }
    }

    template <typename Mode>
    std::optional<BlockVariant> getBlock(Category category, Mode mode, int type) const
    {
        if constexpr (std::is_same_v<Mode, ViewMode>)
        {
            if (category != Category::View)
                return std::nullopt;
            auto mit = viewBlocks_.find(mode);
            if (mit != viewBlocks_.end())
            {
                auto tit = mit->second.find(type);
                if (tit != mit->second.end())
                    return tit->second;
            }
        }
        else if constexpr (std::is_same_v<Mode, LogicMode>)
        {
            if (category != Category::Logic)
                return std::nullopt;
            auto mit = logicBlocks_.find(mode);
            if (mit != logicBlocks_.end())
            {
                auto tit = mit->second.find(type);
                if (tit != mit->second.end())
                    return tit->second;
            }
        }
        return std::nullopt;
    }

private:
    ModeMap<ViewMode> viewBlocks_;
    ModeMap<LogicMode> logicBlocks_;
};

int main()
{
    BlockRegistry reg;

    reg.registerBlock(Category::View, ViewMode::Top, 1, TopBlock1{100, "Main View"});
    reg.registerBlock(Category::Logic, LogicMode::Schematic, 2, SchematicBlock1{5.0, true});

    auto blockOpt = reg.getBlock(Category::View, ViewMode::Top, 1);
    if (blockOpt)
    {
        std::visit([](auto &&blk)
                   {
      using T = std::decay_t<decltype(blk)>;
      if constexpr (std::is_same_v<T, TopBlock1>)
        std::cout << "TopBlock1, label = " << blk.label << ", width = " << blk.width << "\n"; }, *blockOpt);
    }

    return 0;
}
