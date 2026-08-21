// ============================================================
// 医疗管理系统 - 公共工具函数实现
// ============================================================
#include "common.h"

namespace hm {

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) ++b;
    while (e > b && std::isspace((unsigned char)s[e - 1])) --e;
    return s.substr(b, e - b);
}

std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return r;
}

bool containsIgnoreCase(const std::string& hay, const std::string& needle) {
    return toLower(hay).find(toLower(needle)) != std::string::npos;
}

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == sep) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

std::string join(const std::vector<std::string>& v, char sep) {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) out.push_back(sep);
        out += v[i];
    }
    return out;
}

std::string today() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string nowTime() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M");
    return oss.str();
}

std::string sanitize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '|' || c == ';' || c == ',' || c == '\n' || c == '\r' || c == '\t') continue;
        out.push_back(c);
    }
    return out;
}

double toDouble(const std::string& s, double dflt) {
    try {
        size_t pos = 0;
        double v = std::stod(trim(s), &pos);
        if (pos == trim(s).size()) return v;
    } catch (...) {}
    return dflt;
}

int toInt(const std::string& s, int dflt) {
    try {
        size_t pos = 0;
        int v = std::stoi(trim(s), &pos);
        if (pos == trim(s).size()) return v;
    } catch (...) {}
    return dflt;
}

std::string fmtMoney(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << v;
    return oss.str();
}

size_t utf8Len(const std::string& s) {
    size_t n = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if ((c & 0xC0) != 0x80) ++n; // 跳过 UTF-8 连续字节
    }
    return n;
}

int dispWidth(const std::string& s) {
    int w = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 0x80) {
            while (i + 1 < s.size() && ((unsigned char)s[i + 1] & 0xC0) == 0x80) ++i;
            w += 2; // CJK 字符按 2 列
        } else {
            ++w;
        }
    }
    return w;
}

std::string padRight(const std::string& s, int width) {
    int w = dispWidth(s);
    if (w >= width) return s;
    return s + std::string(width - w, ' ');
}

int inputInt(const std::string& prompt, int min, int max) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
    if (std::cin.eof()) return min;
        line = trim(line);
        if (line.empty()) {
            std::cout << "  请输入 " << min << "~" << max << " 之间的整数。\n";
            continue;
        }
        try {
            size_t pos = 0;
            int v = std::stoi(line, &pos);
            if (pos == line.size() && v >= min && v <= max) return v;
        } catch (...) {}
        std::cout << "  请输入 " << min << "~" << max << " 之间的整数。\n";
    }
}

std::string inputLine(const std::string& prompt, int maxChars, bool allowEmpty) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
    if (std::cin.eof()) return "";
        line = trim(sanitize(line));
        if (line.empty() && allowEmpty) return line;
        if (line.empty()) {
            std::cout << "  输入不能为空，请重新输入。\n";
            continue;
        }
        if ((int)utf8Len(line) > maxChars) {
            std::cout << "  输入过长（最多 " << maxChars << " 个字符），请重新输入。\n";
            continue;
        }
        return line;
    }
}

std::string inputYesNo(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " (y/n)：";
        std::string line;
        std::getline(std::cin, line);
    if (std::cin.eof()) return "n";
        line = toLower(trim(line));
        if (line == "y" || line == "yes" || line == "是") return "y";
        if (line == "n" || line == "no" || line == "否") return "n";
        std::cout << "  请输入 y 或 n。\n";
    }
}

void pause() {
    std::cout << "\n按回车键继续...";
    std::string line;
    std::getline(std::cin, line);
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool isValidPhone(const std::string& phone) {
    if (phone.size() != 11) return false;
    if (phone.empty() || phone[0] != '1') return false;
    for (char c : phone)
        if (!std::isdigit((unsigned char)c)) return false;
    return true;
}

std::string inputPhone(const std::string& prompt, bool allowEmpty) {
    while (true) {
        std::string line = inputLine(prompt, MAX_PHONE_LEN, allowEmpty);
        if (line.empty() && allowEmpty) return line;
        if (isValidPhone(line)) return line;
        std::cout << "  手机号须为 11 位数字且以 1 开头，请重新输入。\n";
    }
}


bool isValidIdCard(const std::string& id) {
    if (id.size() != 15 && id.size() != 18) return false;
    for (size_t i = 0; i + 1 < id.size(); ++i)
        if (!std::isdigit((unsigned char)id[i])) return false;
    char last = id.back();
    if (!std::isdigit((unsigned char)last) && last != 'X' && last != 'x') return false;
    int year = 0, month = 0, day = 0;
    try {
        if (id.size() == 18) {
            year = std::stoi(id.substr(6, 4));
            month = std::stoi(id.substr(10, 2));
            day = std::stoi(id.substr(12, 2));
            if (year < 1900 || year > 2100) return false;
        } else {
            year = 1900 + std::stoi(id.substr(6, 2));
            month = std::stoi(id.substr(8, 2));
            day = std::stoi(id.substr(10, 2));
        }
    } catch (...) { return false; }
    if (month < 1 || month > 12 || day < 1) return false;
    static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = mdays[month - 1];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) maxDay = 29;
    return day <= maxDay;
}

std::string inputIdCard(const std::string& prompt) {
    while (true) {
        std::string line = inputLine(prompt, MAX_IDCARD_LEN);
        line = trim(line);
        if (!line.empty() && line.back() == 'x') line.back() = 'X';
        if (isValidIdCard(line)) return line;
        std::cout << "  身份证号须为 15 或 18 位数字（18位末尾可为X），且出生日期须真实。\n";
    }
}

double inputMoney(const std::string& prompt, double maxVal) {
    while (true) {
        std::string line = inputLine(prompt, 12);
        std::string t = trim(line);
        try {
            size_t pos = 0;
            double v = std::stod(t, &pos);
            if (pos == t.size() && std::isfinite(v) && v >= 0 && v <= maxVal) return v;
        } catch (...) {}
        std::cout << "  请输入 0.00 ~ " << fmtMoney(maxVal) << " 之间的金额。\n";
    }
}

bool isValidDate(const std::string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (size_t i = 0; i < d.size(); ++i)
        if (i != 4 && i != 7 && !std::isdigit((unsigned char)d[i])) return false;
    int year = std::stoi(d.substr(0, 4));
    int month = std::stoi(d.substr(5, 2));
    int day = std::stoi(d.substr(8, 2));
    if (year < 2000 || year > 2100) return false;
    if (month < 1 || month > 12 || day < 1) return false;
    static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = mdays[month - 1];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) maxDay = 29;
    return day <= maxDay;
}
} // namespace hm
