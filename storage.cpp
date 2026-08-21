// ============================================================
// 医疗管理系统 - 数据存储层实现
// ============================================================
#include "storage.h"
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace hm {

bool DataStore::ensureDir() {
#ifdef _WIN32
    if (_mkdir(dataDir.c_str()) == 0) return true;
    return errno == EEXIST || _access(dataDir.c_str(), 0) == 0;
#else
    if (mkdir(dataDir.c_str(), 0755) == 0) return true;
    return errno == EEXIST;
#endif
}

static std::string pathOf(const std::string& dir, const std::string& file) {
    return dir + "/" + file;
}

template <typename T>
void DataStore::loadVector(const std::string& file, std::vector<T>& out,
                           std::vector<std::string>& warns,
                           bool (*parser)(const std::string&, T&)) {
    std::ifstream f(pathOf(dataDir, file));
    if (!f.is_open()) return; // 文件不存在视为空（首次运行）
    std::string line;
    int failed = 0;
    while (std::getline(f, line)) {
        if (trim(line).empty()) continue;
        T item;
        if (parser(line, item)) out.push_back(std::move(item));
        else ++failed;
    }
    if (failed > 0)
        warns.push_back(file + "：有 " + std::to_string(failed) + " 行数据损坏，已自动跳过");
}

template <typename T>
void DataStore::saveVector(const std::string& file, const std::vector<T>& items,
                           std::vector<std::string>& errors) {
    std::ofstream f(pathOf(dataDir, file), std::ios::trunc);
    if (!f.is_open()) {
        errors.push_back("无法写入 " + file);
        return;
    }
    for (const auto& it : items) f << it.toLine() << "\n";
}

bool DataStore::loadAll(std::vector<std::string>& warnings) {
    loadVector("patients.txt", patients, warnings, &Patient::fromLine);
    loadVector("doctors.txt", doctors, warnings, &Doctor::fromLine);
    loadVector("nurses.txt", nurses, warnings, &Nurse::fromLine);
    loadVector("pharmacists.txt", pharmacists, warnings, &Pharmacist::fromLine);
    loadVector("admins.txt", admins, warnings, &Admin::fromLine);
    loadVector("departments.txt", departments, warnings, &Department::fromLine);
    loadVector("registrations.txt", registrations, warnings, &Registration::fromLine);
    loadVector("records.txt", records, warnings, &MedicalRecord::fromLine);
    loadVector("exams.txt", exams, warnings, &Examination::fromLine);
    loadVector("reports.txt", reports, warnings, &Report::fromLine);
    loadVector("prescriptions.txt", prescriptions, warnings, &Prescription::fromLine);
    loadVector("wards.txt", wards, warnings, &Ward::fromLine);
    loadVector("beds.txt", beds, warnings, &Bed::fromLine);
    loadVector("hospitalizations.txt", hospitalizations, warnings, &Hospitalization::fromLine);
    loadVector("nursing.txt", nursingRecords, warnings, &NursingRecord::fromLine);
    loadVector("drugs.txt", drugs, warnings, &Drug::fromLine);
    loadVector("stock.txt", stockRecords, warnings, &StockRecord::fromLine);
    loadVector("bills.txt", bills, warnings, &Bill::fromLine);
    initCounters();
    return true;
}

bool DataStore::saveAll(std::vector<std::string>& errors) {
    saveVector("patients.txt", patients, errors);
    saveVector("doctors.txt", doctors, errors);
    saveVector("nurses.txt", nurses, errors);
    saveVector("pharmacists.txt", pharmacists, errors);
    saveVector("admins.txt", admins, errors);
    saveVector("departments.txt", departments, errors);
    saveVector("registrations.txt", registrations, errors);
    saveVector("records.txt", records, errors);
    saveVector("exams.txt", exams, errors);
    saveVector("reports.txt", reports, errors);
    saveVector("prescriptions.txt", prescriptions, errors);
    saveVector("wards.txt", wards, errors);
    saveVector("beds.txt", beds, errors);
    saveVector("hospitalizations.txt", hospitalizations, errors);
    saveVector("nursing.txt", nursingRecords, errors);
    saveVector("drugs.txt", drugs, errors);
    saveVector("stock.txt", stockRecords, errors);
    saveVector("bills.txt", bills, errors);
    return errors.empty();
}

bool DataStore::isEmpty() const {
    return patients.empty() && doctors.empty() && drugs.empty() && departments.empty() &&
           wards.empty() && registrations.empty() && bills.empty();
}

void DataStore::refreshCountersFrom(const std::vector<std::string>& ids) {
    for (const auto& id : ids) {
        for (const auto& kv : _counters) {
            const std::string& prefix = kv.first;
            if (id.size() > prefix.size() && id.compare(0, prefix.size(), prefix) == 0) {
                bool numeric = true;
                for (size_t i = prefix.size(); i < id.size(); ++i)
                    if (!std::isdigit((unsigned char)id[i])) { numeric = false; break; }
                if (numeric) {
                    int n = toInt(id.substr(prefix.size()));
                    if (n > _counters[prefix]) _counters[prefix] = n;
                }
            }
        }
    }
}

void DataStore::initCounters() {
    _counters.clear();
    const std::vector<std::string> prefixes = {"P", "D", "N", "PH", "A", "DEPT", "REG",
                                               "REC", "EX", "RP", "RX", "WARD", "HOS",
                                               "NR", "DRUG", "ST", "BILL"};
    for (const auto& p : prefixes) _counters[p] = 0;

    std::vector<std::string> ids;
    for (const auto& x : patients) ids.push_back(x.id);
    for (const auto& x : doctors) ids.push_back(x.id);
    for (const auto& x : nurses) ids.push_back(x.id);
    for (const auto& x : pharmacists) ids.push_back(x.id);
    for (const auto& x : admins) ids.push_back(x.id);
    for (const auto& x : departments) ids.push_back(x.id);
    for (const auto& x : registrations) ids.push_back(x.id);
    for (const auto& x : records) ids.push_back(x.id);
    for (const auto& x : exams) ids.push_back(x.id);
    for (const auto& x : reports) ids.push_back(x.id);
    for (const auto& x : prescriptions) ids.push_back(x.id);
    for (const auto& x : wards) ids.push_back(x.id);
    for (const auto& x : hospitalizations) ids.push_back(x.id);
    for (const auto& x : nursingRecords) ids.push_back(x.id);
    for (const auto& x : drugs) ids.push_back(x.id);
    for (const auto& x : stockRecords) ids.push_back(x.id);
    for (const auto& x : bills) ids.push_back(x.id);
    refreshCountersFrom(ids);
}

std::string DataStore::nextId(const std::string& prefix) {
    int n = ++_counters[prefix];
    std::string s = std::to_string(n);
    while (s.size() < 5) s = "0" + s;
    return prefix + s;
}

std::string DataStore::nextBedId(const std::string& wardId) {
    int maxN = 0;
    for (const auto& b : beds) {
        if (b.wardId == wardId && b.id.size() > wardId.size() + 1) {
            int n = toInt(b.id.substr(wardId.size() + 1));
            if (n > maxN) maxN = n;
        }
    }
    std::string s = std::to_string(maxN + 1);
    while (s.size() < 2) s = "0" + s;
    return wardId + "-" + s;
}

} // namespace hm
