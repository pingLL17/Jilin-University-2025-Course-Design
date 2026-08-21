#pragma once
// ============================================================
// 医疗管理系统 - 数据存储层（文本文件持久化）
// ============================================================
#include <map>
#include <string>
#include <vector>
#include "models.h"

namespace hm {

class DataStore {
public:
    std::string dataDir = "data";

    std::vector<Patient> patients;
    std::vector<Doctor> doctors;
    std::vector<Nurse> nurses;
    std::vector<Pharmacist> pharmacists;
    std::vector<Admin> admins;
    std::vector<Department> departments;
    std::vector<Registration> registrations;
    std::vector<MedicalRecord> records;
    std::vector<Examination> exams;
    std::vector<Report> reports;
    std::vector<Prescription> prescriptions;
    std::vector<Ward> wards;
    std::vector<Bed> beds;
    std::vector<Hospitalization> hospitalizations;
    std::vector<NursingRecord> nursingRecords;
    std::vector<Drug> drugs;
    std::vector<StockRecord> stockRecords;
    std::vector<Bill> bills;

    bool ensureDir();
    bool loadAll(std::vector<std::string>& warnings);
    bool saveAll(std::vector<std::string>& errors);
    bool isEmpty() const;

    // 生成下一个递增编号（如 P00005）
    std::string nextId(const std::string& prefix);
    // 病房内下一个床位号（如 W001-03）
    std::string nextBedId(const std::string& wardId);

private:
    std::map<std::string, int> _counters;

    void initCounters();
    void refreshCountersFrom(const std::vector<std::string>& ids);
    template <typename T>
    void loadVector(const std::string& file, std::vector<T>& out,
                    std::vector<std::string>& warns,
                    bool (*parser)(const std::string&, T&));
    template <typename T>
    void saveVector(const std::string& file, const std::vector<T>& items,
                    std::vector<std::string>& errors);
};

} // namespace hm
