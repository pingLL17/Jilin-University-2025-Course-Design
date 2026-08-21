#pragma once
// ============================================================
// 医疗管理系统 - 演示数据生成
// ============================================================
#include <string>
#include <vector>
#include "storage.h"

namespace hm {

// 首次运行（数据为空）时生成满足规模要求的演示数据
bool seedIfEmpty(DataStore& store, std::vector<std::string>& msgs);

} // namespace hm
