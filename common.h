#pragma once
// ============================================================
// 医疗管理系统 - 公共常量与工具函数
// ============================================================
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace hm {

// ---------- 存储长度限制（按字符数） ----------
constexpr int MAX_NAME_LEN = 20;        // 姓名最大字符数
constexpr int MAX_DEPT_NAME_LEN = 20;   // 科室名最大字符数
constexpr int MAX_DRUG_NAME_LEN = 40;   // 药品通用名/商品名最大字符数
constexpr int MAX_ALIAS_LEN = 60;       // 药品别名（多个别名用 / 分隔）
constexpr int MAX_TEXT_LEN = 200;       // 病历/医嘱/备注等文本最大字符数
constexpr int MAX_ADDR_LEN = 80;        // 地址最大字符数
constexpr int MAX_IDCARD_LEN = 18;      // 身份证号长度
constexpr int MAX_PHONE_LEN = 20;       // 电话最大字符数
constexpr int MAX_USERNAME_LEN = 20;    // 登录账号最大字符数
constexpr int MAX_PASSWORD_LEN = 20;    // 密码最大字符数

// ---------- 挂号状态 ----------
inline const std::string RS_WAIT = "待就诊";
inline const std::string RS_DONE = "已就诊";
inline const std::string RS_CANCEL = "已取消";

// ---------- 医疗记录类型 ----------
inline const std::string RT_REG = "挂号";
inline const std::string RT_CONSULT = "看诊";
inline const std::string RT_EXAM = "检查";
inline const std::string RT_HOSP = "住院";

// ---------- 床位状态 ----------
inline const std::string BS_FREE = "空闲";
inline const std::string BS_OCCUPIED = "占用";
inline const std::string BS_CLEANING = "清洁中";
inline const std::string BS_REPAIR = "维修中";

// ---------- 病房类型 ----------
inline const std::string WT_GENERAL = "普通";
inline const std::string WT_ICU = "ICU";
inline const std::string WT_ISOLATION = "隔离";

// ---------- 出入库类型 ----------
inline const std::string ST_IN = "入库";
inline const std::string ST_DISPENSE = "发药";
inline const std::string ST_RETURN = "退货";
inline const std::string ST_LOSS = "损耗";

// ---------- 处方状态 ----------
inline const std::string RX_PENDING = "待发药";
inline const std::string RX_DISPENSED = "已发药";
inline const std::string RX_CANCELLED = "已取消";

// ---------- 账单状态 ----------
inline const std::string BI_UNPAID = "待缴费";
inline const std::string BI_PAID = "已缴费";

// ---------- 角色 ----------
inline const std::string ROLE_PATIENT = "患者";
inline const std::string ROLE_DOCTOR = "医生";
inline const std::string ROLE_NURSE = "护士";
inline const std::string ROLE_PHARMACIST = "药剂师";
inline const std::string ROLE_ADMIN = "管理员";

// ---------- 患者状态 ----------
inline const std::string PS_OUTPATIENT = "门诊";
inline const std::string PS_INPATIENT = "住院";

// ---------- 工具函数 ----------
std::string trim(const std::string& s);
std::string toLower(const std::string& s);
bool containsIgnoreCase(const std::string& hay, const std::string& needle);
std::vector<std::string> split(const std::string& s, char sep);
std::string join(const std::vector<std::string>& v, char sep);
std::string today();
std::string nowTime();
std::string sanitize(const std::string& s);
double toDouble(const std::string& s, double dflt = 0.0);
int toInt(const std::string& s, int dflt = 0);
std::string fmtMoney(double v);
size_t utf8Len(const std::string& s);
int dispWidth(const std::string& s);
std::string padRight(const std::string& s, int width);

// 交互输入辅助
int inputInt(const std::string& prompt, int min, int max);
std::string inputLine(const std::string& prompt, int maxChars, bool allowEmpty = false);
bool isValidPhone(const std::string& phone);
std::string inputPhone(const std::string& prompt, bool allowEmpty = false);
bool isValidIdCard(const std::string& id);
std::string inputIdCard(const std::string& prompt);
double inputMoney(const std::string& prompt, double maxVal = 99999.99);
bool isValidDate(const std::string& date);
std::string inputYesNo(const std::string& prompt);
void pause();
void clearScreen();

// 控制台编码设置（Windows 下切换为 UTF-8，保证中文正常显示）
inline void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

} // namespace hm
