#pragma once
// ============================================================
// 医疗管理系统 - 业务服务层（Hospital 门面类）
// ============================================================
#include <string>
#include <vector>
#include "storage.h"

namespace hm {

class Hospital {
public:
    DataStore store;

    // ---- 初始化 ----
    bool init(std::vector<std::string>& warnings);
    void save();
    std::string scaleReport() const;

    // ---- 认证与账号 ----
    const Person* login(const std::string& username, const std::string& password, std::string& err);
    bool patientRegister(Patient& p, std::string& err);
    bool changePassword(const std::string& userId, const std::string& role,
                        const std::string& oldPwd, const std::string& newPwd, std::string& err);
    bool updatePatientProfile(Patient& p, std::string& err);
    bool resetPassword(const std::string& userId, const std::string& role, std::string& err);
    bool addDoctor(Doctor& d, std::string& err);
    bool addDrug(Drug& d, std::string& err);
    bool addWard(Ward& w, std::string& err);
    bool addDepartment(Department& d, std::string& err);
    bool addBed(const std::string& wardId, std::string& err);

    // ---- 查询辅助 ----
    const Department* findDept(const std::string& id) const;
    const Doctor* findDoctor(const std::string& id) const;
    const Patient* findPatient(const std::string& id) const;
    const Nurse* findNurse(const std::string& id) const;
    const Drug* findDrug(const std::string& id) const;
    const Ward* findWard(const std::string& id) const;
    const Bed* findBed(const std::string& id) const;
    const Prescription* findRx(const std::string& id) const;
    const Hospitalization* findHos(const std::string& id) const;
    const Examination* findExam(const std::string& id) const;
    const Report* findReportByExam(const std::string& examId) const;
    std::vector<const Patient*> searchPatients(const std::string& keyword) const;
    std::vector<const Drug*> searchDrugs(const std::string& keyword) const;
    std::vector<const Doctor*> doctorsOfDept(const std::string& deptId) const;
    std::vector<const Bed*> bedsOfWard(const std::string& wardId) const;
    std::vector<const Ward*> wardsOfType(const std::string& type) const;

    // ---- 挂号 ----
    std::string createRegistration(const std::string& patientId, const std::string& doctorId,
                                   const std::string& date, const std::string& slot,
                                   const std::string& type, std::string& err);
    bool cancelRegistration(const std::string& regId, std::string& err);

    // ---- 看诊 ----
    bool consult(const std::string& regId, const std::string& chief, const std::string& diagnosis,
                 const std::string& advice, std::string& err);

    // ---- 检查与报告 ----
    std::string createExamination(const std::string& patientId, const std::string& doctorId,
                                  const std::string& deptId, const std::string& category,
                                  const std::string& items, double fee, std::string& err);
    bool addReport(const std::string& examId, const std::string& result, const std::string& conclusion,
                   const std::string& doctorId, std::string& err);

    // ---- 处方与发药 ----
    std::string createPrescription(const std::string& patientId, const std::string& doctorId,
                                   const std::string& deptId,
                                   const std::vector<PrescriptionItem>& items, std::string& err);
    bool dispensePrescription(const std::string& rxId, const std::string& operatorName, std::string& err);
    bool cancelPrescription(const std::string& rxId, std::string& err);
    double rxTotal(const Prescription& rx) const;

    // ---- 药房库存 ----
    bool stockIn(const std::string& drugId, int qty, const std::string& operatorName,
                 const std::string& remark, std::string& err);
    bool stockOut(const std::string& drugId, int qty, const std::string& type,
                  const std::string& operatorName, const std::string& remark, std::string& err);
    std::vector<const Drug*> lowStockDrugs() const;

    // ---- 住院 ----
    std::string admit(const std::string& patientId, const std::string& deptId,
                      const std::string& doctorId, const std::string& wardId, std::string& err);
    bool discharge(const std::string& hosId, const std::string& operatorName, std::string& err);
    std::string addNursingRecord(const std::string& patientId, const std::string& nurseId,
                                 const std::string& content, std::string& err);
    bool setBedStatus(const std::string& bedId, const std::string& status, std::string& err);
    int activeDays(const Hospitalization& h) const;

    // ---- 账单 ----
    bool payBill(const std::string& billId, const std::string& operatorName, std::string& err);
    std::vector<const Bill*> billsOfPatient(const std::string& patientId) const;
    double patientUnpaidTotal(const std::string& patientId) const;

    // ---- 报表（患者视角） ----
    std::string reportPatientInfo(const std::string& patientId) const;
    std::string reportPatientRecords(const std::string& patientId) const;
    std::string reportPatientBills(const std::string& patientId) const;
    std::string reportPatientExams(const std::string& patientId) const;
    std::string reportPatientRx(const std::string& patientId) const;
    std::string reportPatientHos(const std::string& patientId) const;

    // ---- 报表（医护视角） ----
    std::string reportDailyOutpatient() const;
    std::string reportDoctorWorkload() const;
    std::string reportBedOccupancy() const;
    std::string reportLowStock() const;
    std::string reportPendingRx() const;
    std::string reportDoctorPatients(const std::string& doctorId) const;

    // ---- 报表（管理视角） ----
    std::string reportDeptRevenue() const;
    std::string reportDrugFlow() const;
    std::string reportDrugSales() const;
    std::string reportBedTurnover() const;
    std::string reportPatientFlow() const;
    std::string reportAll() const;

    // ---- 基础信息查询 ----
    std::string reportByPatient(const std::string& keyword) const;
    std::string reportByDoctor(const std::string& keyword) const;
    std::string reportByDept(const std::string& keyword) const;
    std::string reportByDrug(const std::string& keyword) const;

private:
    std::string _newBill(const std::string& patientId, const std::vector<BillItem>& items);
    void _addBillItem(const std::string& billId, const BillItem& item);
    int _daysBetween(const std::string& a, const std::string& b) const;
};

} // namespace hm
