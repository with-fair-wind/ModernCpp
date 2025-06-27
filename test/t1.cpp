#include "t1.h"

Convert &Convert::instance()
{
    // TODO: 在此处插入 return 语句
    {
        static Convert obj;
        return obj;
    }
}