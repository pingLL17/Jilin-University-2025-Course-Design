// ============================================================
// 医疗管理系统（患者视角优先）- 程序入口
// ============================================================
#include <iostream>
#include <vector>
#include "common.h"
#include "services.h"
#include "ui.h"

int main() {
    hm::setupConsole(); // Windows 下设置控制台为 UTF-8，保证中文正常显示

    hm::Hospital h;
    std::vector<std::string> warnings;
    h.init(warnings); // 加载数据；首次运行自动生成演示数据

    for (const auto& w : warnings)
        std::cout << "[提示] " << w << "\n";


    hm::runApp(h);
    return 0;
}
