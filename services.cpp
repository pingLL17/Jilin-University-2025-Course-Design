// ============================================================
// 医疗管理系统 - 业务服务层实现（第1块：初始化/规模校验）
// ============================================================
#include "services.h"
#include "seed.h"
#include <cmath>

namespace hm {

// ---------- 报表排版辅助 ----------
namespace {
std::string rline(const std::vector<std::string>& cells, const std::vector<int>& ws) {
    std::string s;
    for (size_t i = 0; i < cells.size(); ++i)
        s += padRight(cells[i], ws[i]) + "  ";
    return s;
}
std::string rsep(const std::vector<int>& ws) {
    std::string s;
    for (int w : ws) s += std::string(w + 2, '-');
    return s;
}
} // namespace

// ==================== 初始化 ====================

bool Hospital::init(std::vector<std::string>& warnings) {
    store.ensureDir();
    store.loadAll(warnings);
    if (store.isEmpty()) {
        seedIfEmpty(store, warnings);
        std::vector<std::string> errors;
        store.saveAll(errors);
        for (const auto& e : errors) warnings.push_back("保存种子数据失败：" + e);
    }
    return true;
}

void Hospital::save() {
    std::vector<std::string> errors;
    store.saveAll(errors);
    for (const auto& e : errors) std::cout << "  [警告] " << e << "\n";
}

std::string Hospital::scaleReport() const {
    std::string s;
    s += "==================== 数据规模校验 ====================\n";
    int outpatients = 0, inpatients = 0;
    for (const auto& p : store.patients) {
        if (p.status == PS_INPATIENT) ++inpatients; else ++outpatients;
    }
    auto pass = [&](bool ok, const std::string& line) {
        s += std::string(ok ? "[达标] " : "[未达] ") + line + "\n";
    };
    pass(outpatients >= 100, "门诊患者 >= 100（当前 " + std::to_string(outpatients) + " 名）");
    pass(inpatients >= 30, "住院患者 >= 30（当前 " + std::to_string(inpatients) + " 名）");
    pass((int)store.doctors.size() >= 20, "医生 >= 20（当前 " + std::to_string(store.doctors.size()) + " 名）");
    pass((int)store.departments.size() >= 5, "科室 >= 5（当前 " + std::to_string(store.departments.size()) + " 个）");
    bool deptOk = true;
    std::string deptDetail;
    for (const auto& d : store.departments) {
        int n = 0;
        for (const auto& doc : store.doctors) if (doc.deptId == d.id) ++n;
        deptDetail += "  " + d.name + "(" + std::to_string(n) + "医)";
        if (n < 3) deptOk = false;
    }
    pass(deptOk && (int)store.departments.size() >= 5, "每科室医生 >= 3：" + deptDetail);
    bool wtG = false, wtI = false, wtS = false;
    for (const auto& w : store.wards) {
        if (w.type == WT_GENERAL) wtG = true;
        if (w.type == WT_ICU) wtI = true;
        if (w.type == WT_ISOLATION) wtS = true;
    }
    pass(wtG && wtI && wtS, "病房类型 >= 3 种（普通/ICU/隔离）");
    int deptLinkedWards = 0;
    for (const auto& w : store.wards) if (!w.deptId.empty()) ++deptLinkedWards;
    pass(deptLinkedWards >= 1, "部分病房与科室关联（当前 " + std::to_string(deptLinkedWards) + " 间关联科室）");
    pass((int)store.drugs.size() >= 20, "药品 >= 20 种（当前 " + std::to_string(store.drugs.size()) + " 种）");
    int deptLinkedDrugs = 0;
    for (const auto& d : store.drugs) if (!d.deptId.empty()) ++deptLinkedDrugs;
    pass(deptLinkedDrugs >= 1, "部分药品与科室相关（当前 " + std::to_string(deptLinkedDrugs) + " 种关联科室）");
    bool dup = false;
    for (size_t i = 0; i < store.patients.size() && !dup; ++i)
        for (size_t j = i + 1; j < store.patients.size(); ++j)
            if (store.patients[i].name == store.patients[j].name) { dup = true; break; }
    pass(dup, "存在同名患者（重名场景已覆盖）");
    s += "=====================================================\n";
    return s;
}

// ==================== 认证与账号（第2块） ====================

const Person* Hospital::login(const std::string& username, const std::string& password, std::string& err) {
    auto check = [&](const Person& p) -> bool {
        return p.username == username && p.password == password;
    };
    for (const auto& p : store.patients) if (check(p)) return &p;
    for (const auto& p : store.doctors) if (check(p)) return &p;
    for (const auto& p : store.nurses) if (check(p)) return &p;
    for (const auto& p : store.pharmacists) if (check(p)) return &p;
    for (const auto& p : store.admins) if (check(p)) return &p;
    err = "账号或密码错误";
    return nullptr;
}

static bool usernameExists(const DataStore& store, const std::string& username) {
    auto has = [&](const Person& p) { return p.username == username; };
    for (const auto& p : store.patients) if (has(p)) return true;
    for (const auto& p : store.doctors) if (has(p)) return true;
    for (const auto& p : store.nurses) if (has(p)) return true;
    for (const auto& p : store.pharmacists) if (has(p)) return true;
    for (const auto& p : store.admins) if (has(p)) return true;
    return false;
}

bool Hospital::patientRegister(Patient& p, std::string& err) {
    if (!isValidIdCard(p.idCard)) {
        err = "身份证号格式不正确（须为 15 或 18 位且出生日期真实）";
        return false;
    }
    for (const auto& x : store.patients) {
        if (x.idCard == p.idCard) { err = "该身份证号已建档，不能重复注册"; return false; }
        if (!x.username.empty() && x.username == p.username) { err = "该登录账号已被占用"; return false; }
    }
    if (usernameExists(store, p.username)) { err = "该登录账号已被占用"; return false; }
    p.id = store.nextId("P");
    p.role = ROLE_PATIENT;
    p.regDate = today();
    if (p.status.empty()) p.status = PS_OUTPATIENT;
    store.patients.push_back(p);
    save();
    return true;
}

bool Hospital::changePassword(const std::string& userId, const std::string& role,
                              const std::string& oldPwd, const std::string& newPwd, std::string& err) {
    if (newPwd.size() < 4 || (int)newPwd.size() > MAX_PASSWORD_LEN) {
        err = "新密码长度需为 4~" + std::to_string(MAX_PASSWORD_LEN) + " 位";
        return false;
    }
    auto tryChange = [&](Person& p) -> bool {
        if (p.id != userId) return false;
        if (p.password != oldPwd) { err = "原密码不正确"; return false; }
        p.password = newPwd;
        save();
        return true;
    };
    if (role == ROLE_PATIENT) for (auto& p : store.patients) if (tryChange(p)) return true;
    if (role == ROLE_DOCTOR) for (auto& p : store.doctors) if (tryChange(p)) return true;
    if (role == ROLE_NURSE) for (auto& p : store.nurses) if (tryChange(p)) return true;
    if (role == ROLE_PHARMACIST) for (auto& p : store.pharmacists) if (tryChange(p)) return true;
    if (role == ROLE_ADMIN) for (auto& p : store.admins) if (tryChange(p)) return true;
    if (err.empty()) err = "未找到该用户";
    return false;
}

bool Hospital::updatePatientProfile(Patient& p, std::string& err) {
    for (auto& x : store.patients) {
        if (x.id == p.id) {
            x.phone = p.phone;
            x.address = p.address;
            x.bloodType = p.bloodType;
            x.allergy = p.allergy;
            save();
            return true;
        }
    }
    err = "未找到该患者";
    return false;
}

bool Hospital::resetPassword(const std::string& userId, const std::string& role, std::string& err) {
    auto doReset = [&](Person& p) { p.password = "123456"; };
    if (role == ROLE_PATIENT) for (auto& p : store.patients) if (p.id == userId) { doReset(p); save(); return true; }
    if (role == ROLE_DOCTOR) for (auto& p : store.doctors) if (p.id == userId) { doReset(p); save(); return true; }
    if (role == ROLE_NURSE) for (auto& p : store.nurses) if (p.id == userId) { doReset(p); save(); return true; }
    if (role == ROLE_PHARMACIST) for (auto& p : store.pharmacists) if (p.id == userId) { doReset(p); save(); return true; }
    if (role == ROLE_ADMIN) for (auto& p : store.admins) if (p.id == userId) { doReset(p); save(); return true; }
    err = "未找到该用户";
    return false;
}

bool Hospital::addDoctor(Doctor& d, std::string& err) {
    if (usernameExists(store, d.username)) { err = "该登录账号已被占用"; return false; }
    d.id = store.nextId("D");
    d.role = ROLE_DOCTOR;
    if (d.password.empty()) d.password = "123456";
    store.doctors.push_back(d);
    save();
    return true;
}

bool Hospital::addDrug(Drug& d, std::string& err) {
    (void)err;
    d.id = store.nextId("DRUG");
    store.drugs.push_back(d);
    if (!d.deptId.empty()) {
        for (auto& dep : store.departments) {
            if (dep.id == d.deptId && std::find(dep.drugIds.begin(), dep.drugIds.end(), d.id) == dep.drugIds.end())
                dep.drugIds.push_back(d.id);
        }
    }
    save();
    return true;
}

bool Hospital::addWard(Ward& w, std::string& err) {
    (void)err;
    w.id = store.nextId("WARD");
    store.wards.push_back(w);
    save();
    return true;
}

bool Hospital::addDepartment(Department& d, std::string& err) {
    (void)err;
    d.id = store.nextId("DEPT");
    store.departments.push_back(d);
    save();
    return true;
}

bool Hospital::addBed(const std::string& wardId, std::string& err) {
    const Ward* w = findWard(wardId);
    if (!w) { err = "未找到该病房"; return false; }
    Bed b;
    b.id = store.nextBedId(wardId);
    b.wardId = wardId;
    b.status = BS_FREE;
    store.beds.push_back(b);
    save();
    return true;
}

// ==================== 查询辅助（第3块） ====================

const Department* Hospital::findDept(const std::string& id) const {
    for (const auto& d : store.departments) if (d.id == id) return &d;
    return nullptr;
}
const Doctor* Hospital::findDoctor(const std::string& id) const {
    for (const auto& d : store.doctors) if (d.id == id) return &d;
    return nullptr;
}
const Patient* Hospital::findPatient(const std::string& id) const {
    for (const auto& p : store.patients) if (p.id == id) return &p;
    return nullptr;
}
const Nurse* Hospital::findNurse(const std::string& id) const {
    for (const auto& n : store.nurses) if (n.id == id) return &n;
    return nullptr;
}
const Drug* Hospital::findDrug(const std::string& id) const {
    for (const auto& d : store.drugs) if (d.id == id) return &d;
    return nullptr;
}
const Ward* Hospital::findWard(const std::string& id) const {
    for (const auto& w : store.wards) if (w.id == id) return &w;
    return nullptr;
}
const Bed* Hospital::findBed(const std::string& id) const {
    for (const auto& b : store.beds) if (b.id == id) return &b;
    return nullptr;
}
const Prescription* Hospital::findRx(const std::string& id) const {
    for (const auto& r : store.prescriptions) if (r.id == id) return &r;
    return nullptr;
}
const Hospitalization* Hospital::findHos(const std::string& id) const {
    for (const auto& h : store.hospitalizations) if (h.id == id) return &h;
    return nullptr;
}
const Examination* Hospital::findExam(const std::string& id) const {
    for (const auto& e : store.exams) if (e.id == id) return &e;
    return nullptr;
}
const Report* Hospital::findReportByExam(const std::string& examId) const {
    for (const auto& r : store.reports) if (r.examId == examId) return &r;
    return nullptr;
}

std::vector<const Patient*> Hospital::searchPatients(const std::string& keyword) const {
    std::vector<const Patient*> out;
    for (const auto& p : store.patients)
        if (containsIgnoreCase(p.name, keyword) || containsIgnoreCase(p.id, keyword) ||
            containsIgnoreCase(p.idCard, keyword))
            out.push_back(&p);
    return out;
}

std::vector<const Drug*> Hospital::searchDrugs(const std::string& keyword) const {
    std::vector<const Drug*> out;
    for (const auto& d : store.drugs)
        if (containsIgnoreCase(d.genericName, keyword) || containsIgnoreCase(d.brandName, keyword) ||
            containsIgnoreCase(d.alias, keyword) || containsIgnoreCase(d.id, keyword))
            out.push_back(&d);
    return out;
}

std::vector<const Doctor*> Hospital::doctorsOfDept(const std::string& deptId) const {
    std::vector<const Doctor*> out;
    for (const auto& d : store.doctors) if (d.deptId == deptId) out.push_back(&d);
    return out;
}

std::vector<const Bed*> Hospital::bedsOfWard(const std::string& wardId) const {
    std::vector<const Bed*> out;
    for (const auto& b : store.beds) if (b.wardId == wardId) out.push_back(&b);
    return out;
}

std::vector<const Ward*> Hospital::wardsOfType(const std::string& type) const {
    std::vector<const Ward*> out;
    for (const auto& w : store.wards) if (w.type == type) out.push_back(&w);
    return out;
}

// ==================== 挂号 ====================

std::string Hospital::createRegistration(const std::string& patientId, const std::string& doctorId,
                                         const std::string& date, const std::string& slot,
                                         const std::string& type, std::string& err) {
    const Doctor* doc = findDoctor(doctorId);
    if (!doc) { err = "未找到该医生"; return ""; }
    const Patient* p = findPatient(patientId);
    if (!p) { err = "未找到该患者"; return ""; }
    Registration r;
    r.id = store.nextId("REG");
    r.patientId = patientId;
    r.doctorId = doctorId;
    r.deptId = doc->deptId;
    r.date = date;
    r.slot = slot;
    r.type = type;
    r.status = RS_WAIT;
    int maxSeq = 0;
    for (const auto& x : store.registrations)
        if (x.doctorId == doctorId && x.date == date && x.slot == slot && x.seq > maxSeq) maxSeq = x.seq;
    r.seq = maxSeq + 1;
    r.fee = doc->title.find("主任") != std::string::npos ? 30.0 : 10.0;
    store.registrations.push_back(r);

    MedicalRecord rec;
    rec.id = store.nextId("REC");
    rec.patientId = patientId;
    rec.doctorId = doctorId;
    rec.deptId = doc->deptId;
    rec.type = RT_REG;
    rec.date = date;
    rec.time = nowTime();
    rec.chief = type + "挂号（" + doc->name + " " + slot + "）";
    rec.advice = "";
    rec.diagnosis = "";
    rec.linkedId = r.id;
    store.records.push_back(rec);

    BillItem it;
    it.category = "挂号费";
    it.desc = doc->name + "(" + doc->title + ") 挂号";
    it.amount = r.fee;
    _newBill(patientId, {it});
    save();
    return r.id;
}

bool Hospital::cancelRegistration(const std::string& regId, std::string& err) {
    for (auto& r : store.registrations) {
        if (r.id == regId) {
            if (r.status != RS_WAIT) { err = "该挂号单当前状态不可取消"; return false; }
            r.status = RS_CANCEL;
            save();
            return true;
        }
    }
    err = "未找到该挂号单";
    return false;
}

// ==================== 看诊 ====================

bool Hospital::consult(const std::string& regId, const std::string& chief, const std::string& diagnosis,
                       const std::string& advice, std::string& err) {
    for (auto& r : store.registrations) {
        if (r.id == regId) {
            if (r.status != RS_WAIT) { err = "该挂号单不是待就诊状态"; return false; }
            r.status = RS_DONE;
            MedicalRecord rec;
            rec.id = store.nextId("REC");
            rec.patientId = r.patientId;
            rec.doctorId = r.doctorId;
            rec.deptId = r.deptId;
            rec.type = RT_CONSULT;
            rec.date = r.date;
            rec.time = nowTime();
            rec.chief = chief;
            rec.diagnosis = diagnosis;
            rec.advice = advice;
            rec.linkedId = r.id;
            store.records.push_back(rec);
            save();
            return true;
        }
    }
    err = "未找到该挂号单";
    return false;
}

// ==================== 检查与报告（第4块） ====================

std::string Hospital::createExamination(const std::string& patientId, const std::string& doctorId,
                                        const std::string& deptId, const std::string& category,
                                        const std::string& items, double fee, std::string& err) {
    if (fee < 0) { err = "检查费用不能为负"; return ""; }
    Examination e;
    e.id = store.nextId("EX");
    e.patientId = patientId;
    e.doctorId = doctorId;
    e.deptId = deptId;
    e.category = category;
    e.items = items;
    e.date = today();
    e.status = "待检查";
    e.fee = fee;
    store.exams.push_back(e);

    MedicalRecord rec;
    rec.id = store.nextId("REC");
    rec.patientId = patientId;
    rec.doctorId = doctorId;
    rec.deptId = deptId;
    rec.type = RT_EXAM;
    rec.date = e.date;
    rec.time = nowTime();
    rec.chief = "检查申请：" + category + "（" + items + "）";
    rec.diagnosis = "";
    rec.advice = "";
    rec.linkedId = e.id;
    store.records.push_back(rec);

    BillItem it;
    it.category = "检查费";
    it.desc = category + " " + items;
    it.amount = fee;
    _newBill(patientId, {it});
    save();
    return e.id;
}

bool Hospital::addReport(const std::string& examId, const std::string& result, const std::string& conclusion,
                         const std::string& doctorId, std::string& err) {
    for (auto& e : store.exams) {
        if (e.id == examId) {
            if (findReportByExam(examId)) { err = "该检查已有报告，不能重复录入"; return false; }
            Report rp;
            rp.id = store.nextId("RP");
            rp.examId = examId;
            rp.result = result;
            rp.conclusion = conclusion;
            rp.date = today();
            rp.doctorId = doctorId;
            store.reports.push_back(rp);
            e.status = "已完成";
            save();
            return true;
        }
    }
    err = "未找到该检查单";
    return false;
}

// ==================== 处方与发药 ====================

double Hospital::rxTotal(const Prescription& rx) const {
    double s = 0;
    for (const auto& it : rx.items) {
        const Drug* d = findDrug(it.drugId);
        if (d) s += d->price * it.qty;
    }
    return s;
}

std::string Hospital::createPrescription(const std::string& patientId, const std::string& doctorId,
                                         const std::string& deptId,
                                         const std::vector<PrescriptionItem>& items, std::string& err) {
    if (items.empty()) { err = "处方不能为空"; return ""; }
    double total = 0;
    for (const auto& it : items) {
        const Drug* d = findDrug(it.drugId);
        if (!d) { err = "处方中有不存在的药品"; return ""; }
        if (it.qty <= 0) { err = "药品数量必须为正数"; return ""; }
        if (d->stock < it.qty) { err = "药品[" + d->genericName + "]库存不足（当前 " + std::to_string(d->stock) + "）"; return ""; }
        total += d->price * it.qty;
    }
    Prescription rx;
    rx.id = store.nextId("RX");
    rx.patientId = patientId;
    rx.doctorId = doctorId;
    rx.deptId = deptId;
    rx.date = today();
    rx.status = RX_PENDING;
    rx.items = items;
    store.prescriptions.push_back(rx);

    BillItem it;
    it.category = "药费";
    it.desc = "处方 " + rx.id + " 药品费用";
    it.amount = total;
    _newBill(patientId, {it});
    save();
    return rx.id;
}

bool Hospital::dispensePrescription(const std::string& rxId, const std::string& operatorName, std::string& err) {
    for (auto& rx : store.prescriptions) {
        if (rx.id == rxId) {
            if (rx.status != RX_PENDING) { err = "该处方不是待发药状态"; return false; }
            for (const auto& it : rx.items) {
                for (auto& d : store.drugs) {
                    if (d.id == it.drugId) {
                        if (d.stock < it.qty) { err = "药品[" + d.genericName + "]库存不足，无法发药"; return false; }
                        d.stock -= it.qty;
                    }
                }
            }
            rx.status = RX_DISPENSED;
            for (const auto& it : rx.items) {
                StockRecord sr;
                sr.id = store.nextId("ST");
                sr.drugId = it.drugId;
                sr.type = ST_DISPENSE;
                sr.date = today();
                sr.operatorName = operatorName;
                sr.remark = "处方发药";
                sr.refId = rxId;
                sr.qty = it.qty;
                store.stockRecords.push_back(sr);
            }
            save();
            return true;
        }
    }
    err = "未找到该处方";
    return false;
}

bool Hospital::cancelPrescription(const std::string& rxId, std::string& err) {
    for (auto& rx : store.prescriptions) {
        if (rx.id == rxId) {
            if (rx.status != RX_PENDING) { err = "仅待发药状态的处方可取消"; return false; }
            rx.status = RX_CANCELLED;
            save();
            return true;
        }
    }
    err = "未找到该处方";
    return false;
}

// ==================== 药房库存 ====================

bool Hospital::stockIn(const std::string& drugId, int qty, const std::string& operatorName,
                       const std::string& remark, std::string& err) {
    if (qty <= 0) { err = "数量必须为正数"; return false; }
    for (auto& d : store.drugs) {
        if (d.id == drugId) {
            d.stock += qty;
            StockRecord sr;
            sr.id = store.nextId("ST");
            sr.drugId = drugId;
            sr.type = ST_IN;
            sr.date = today();
            sr.operatorName = operatorName;
            sr.remark = remark.empty() ? "药品入库" : remark;
            sr.qty = qty;
            store.stockRecords.push_back(sr);
            save();
            return true;
        }
    }
    err = "未找到该药品";
    return false;
}

bool Hospital::stockOut(const std::string& drugId, int qty, const std::string& type,
                        const std::string& operatorName, const std::string& remark, std::string& err) {
    if (qty <= 0) { err = "数量必须为正数"; return false; }
    for (auto& d : store.drugs) {
        if (d.id == drugId) {
            if (d.stock < qty) { err = "库存不足（当前 " + std::to_string(d.stock) + "）"; return false; }
            d.stock -= qty;
            StockRecord sr;
            sr.id = store.nextId("ST");
            sr.drugId = drugId;
            sr.type = type;
            sr.date = today();
            sr.operatorName = operatorName;
            sr.remark = remark.empty() ? ("药品" + type) : remark;
            sr.qty = qty;
            store.stockRecords.push_back(sr);
            save();
            return true;
        }
    }
    err = "未找到该药品";
    return false;
}

std::vector<const Drug*> Hospital::lowStockDrugs() const {
    std::vector<const Drug*> out;
    for (const auto& d : store.drugs)
        if (d.stock <= d.minStock) out.push_back(&d);
    return out;
}

// ==================== 住院（第5块） ====================

int Hospital::activeDays(const Hospitalization& h) const {
    return _daysBetween(h.admitDate, h.dischargeDate.empty() ? today() : h.dischargeDate);
}

std::string Hospital::admit(const std::string& patientId, const std::string& deptId,
                            const std::string& doctorId, const std::string& wardId, std::string& err) {
    const Patient* p = findPatient(patientId);
    if (!p) { err = "未找到该患者"; return ""; }
    if (p->status == PS_INPATIENT) { err = "该患者已在住院中"; return ""; }
    for (const auto& h : store.hospitalizations)
        if (h.patientId == patientId && h.status == "住院中") { err = "该患者已有未结束的住院单"; return ""; }

    const Ward* ward = findWard(wardId);
    if (!ward) {
        const Department* dep = findDept(deptId);
        if (dep && dep->wardType != "无") {
            const auto& ws = wardsOfType(dep->wardType);
            for (const auto& w : ws) {
                if (w->deptId.empty() || w->deptId == deptId) { ward = w; break; }
            }
        }
        if (!ward) {
            for (const auto& w : store.wards)
                if (w.deptId.empty() || w.deptId == deptId) { ward = &w; break; }
        }
        if (!ward) { err = "未找到合适的病房"; return ""; }
    }
    for (const auto& b : store.beds) {
        if (b.wardId == ward->id && b.status == BS_FREE) {
            for (auto& bed : store.beds) {
                if (bed.id == b.id) {
                    bed.status = BS_OCCUPIED;
                    bed.patientId = patientId;
                }
            }
            Hospitalization h;
            h.id = store.nextId("HOS");
            h.patientId = patientId;
            h.bedId = b.id;
            h.wardId = ward->id;
            h.doctorId = doctorId;
            h.admitDate = today();
            h.status = "住院中";
            h.remark = "";
            store.hospitalizations.push_back(h);

            MedicalRecord rec;
            rec.id = store.nextId("REC");
            rec.patientId = patientId;
            rec.doctorId = doctorId;
            rec.deptId = deptId.empty() ? ward->deptId : deptId;
            rec.type = RT_HOSP;
            rec.date = today();
            rec.time = nowTime();
            rec.chief = "办理入院：" + ward->name + " " + b.id;
            rec.diagnosis = "";
            rec.advice = "住院观察治疗";
            rec.linkedId = h.id;
            store.records.push_back(rec);

            BillItem it;
            it.category = "住院费";
            it.desc = ward->name + " 住院押金";
            it.amount = 1000;
            _newBill(patientId, {it});

            for (auto& pt : store.patients) if (pt.id == patientId) pt.status = PS_INPATIENT;
            save();
            return h.id;
        }
    }
    err = "所选病房暂无空闲床位";
    return "";
}

bool Hospital::discharge(const std::string& hosId, const std::string& operatorName, std::string& err) {
    for (auto& h : store.hospitalizations) {
        if (h.id == hosId) {
            if (h.status != "住院中") { err = "该住院单已结束"; return false; }
            const Ward* ward = findWard(h.wardId);
            int days = std::max(1, activeDays(h));
            double bedFee = ward ? ward->bedFee * days : 0;
            bool appended = false;
            for (auto& b : store.bills) {
                if (b.patientId == h.patientId && b.status == BI_UNPAID) {
                    for (const auto& it : b.items)
                        if (it.category == "住院费") { appended = true; break; }
                    if (appended) {
                        BillItem it;
                        it.category = "床位费";
                        it.desc = (ward ? ward->name : "病房") + " " + std::to_string(days) + " 天 x " +
                                  fmtMoney(ward ? ward->bedFee : 0);
                        it.amount = bedFee;
                        b.items.push_back(it);
                    }
                    break;
                }
            }
            if (!appended && bedFee > 0) {
                BillItem it;
                it.category = "床位费";
                it.desc = (ward ? ward->name : "病房") + " " + std::to_string(days) + " 天 x " +
                          fmtMoney(ward ? ward->bedFee : 0);
                it.amount = bedFee;
                _newBill(h.patientId, {it});
            }
            h.dischargeDate = today();
            h.status = "已出院";
            for (auto& bed : store.beds) {
                if (bed.id == h.bedId) {
                    bed.status = BS_CLEANING;
                    bed.patientId = "";
                }
            }
            for (auto& pt : store.patients) if (pt.id == h.patientId) pt.status = PS_OUTPATIENT;
            (void)operatorName;
            save();
            return true;
        }
    }
    err = "未找到该住院单";
    return false;
}

std::string Hospital::addNursingRecord(const std::string& patientId, const std::string& nurseId,
                                       const std::string& content, std::string& err) {
    const Patient* p = findPatient(patientId);
    if (!p) { err = "未找到该患者"; return ""; }
    if (p->status != PS_INPATIENT) { err = "该患者不在住院状态"; return ""; }
    std::string bedId;
    for (const auto& h : store.hospitalizations)
        if (h.patientId == patientId && h.status == "住院中") { bedId = h.bedId; break; }
    if (bedId.empty()) { err = "未找到该患者的在院床位"; return ""; }
    NursingRecord nr;
    nr.id = store.nextId("NR");
    nr.patientId = patientId;
    nr.bedId = bedId;
    nr.nurseId = nurseId;
    nr.date = today();
    nr.time = nowTime();
    nr.content = content;
    store.nursingRecords.push_back(nr);
    save();
    return nr.id;
}

bool Hospital::setBedStatus(const std::string& bedId, const std::string& status, std::string& err) {
    for (auto& b : store.beds) {
        if (b.id == bedId) {
            if (status == BS_OCCUPIED) { err = "占用状态由入院流程自动设置"; return false; }
            if (!b.patientId.empty()) {
                err = "床位当前有患者占用，请先办理出院"; return false;
            }
            b.status = status;
            save();
            return true;
        }
    }
    err = "未找到该床位";
    return false;
}

// ==================== 账单 ====================

std::string Hospital::_newBill(const std::string& patientId, const std::vector<BillItem>& items) {
    Bill b;
    b.id = store.nextId("BILL");
    b.patientId = patientId;
    b.date = today();
    b.status = BI_UNPAID;
    b.payDate = "";
    b.items = items;
    store.bills.push_back(b);
    return b.id;
}

void Hospital::_addBillItem(const std::string& billId, const BillItem& item) {
    for (auto& b : store.bills) if (b.id == billId) b.items.push_back(item);
}

bool Hospital::payBill(const std::string& billId, const std::string& operatorName, std::string& err) {
    for (auto& b : store.bills) {
        if (b.id == billId) {
            if (b.status != BI_UNPAID) { err = "该账单不是待缴费状态"; return false; }
            b.status = BI_PAID;
            b.payDate = today();
            (void)operatorName;
            save();
            return true;
        }
    }
    err = "未找到该账单";
    return false;
}

std::vector<const Bill*> Hospital::billsOfPatient(const std::string& patientId) const {
    std::vector<const Bill*> out;
    for (const auto& b : store.bills) if (b.patientId == patientId) out.push_back(&b);
    return out;
}

double Hospital::patientUnpaidTotal(const std::string& patientId) const {
    double s = 0;
    for (const auto& b : store.bills)
        if (b.patientId == patientId && b.status == BI_UNPAID) s += b.total();
    return s;
}

// ==================== 报表：患者视角（第6块） ====================

std::string Hospital::reportPatientInfo(const std::string& patientId) const {
    const Patient* p = findPatient(patientId);
    if (!p) return "未找到该患者。\n";
    std::string s;
    s += "================ 患者基本信息 ================\n";
    s += "  患者编号：" + p->id + "\n";
    s += "  姓    名：" + p->name + "\n";
    s += "  性    别：" + p->gender + "\n";
    s += "  年    龄：" + std::to_string(p->age) + "\n";
    s += "  身份证号：" + p->idCard + "\n";
    s += "  联系电话：" + p->phone + "\n";
    s += "  血    型：" + (p->bloodType.empty() ? "未知" : p->bloodType) + "\n";
    s += "  过敏史  ：" + (p->allergy.empty() ? "无" : p->allergy) + "\n";
    s += "  家庭住址：" + (p->address.empty() ? "无" : p->address) + "\n";
    s += "  建档日期：" + p->regDate + "    状态：" + p->status + "\n";
    s += "==============================================\n";
    return s;
}

std::string Hospital::reportPatientRecords(const std::string& patientId) const {
    std::string s = "================ 就诊与医疗记录 ================\n";
    std::vector<int> ws = {11, 8, 10, 10, 22, 22, 22};
    s += rline({"日期", "类型", "医生", "科室", "主诉/内容", "诊断", "医嘱"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int n = 0;
    for (const auto& r : store.records) {
        if (r.patientId != patientId) continue;
        ++n;
        const Doctor* d = findDoctor(r.doctorId);
        const Department* dep = findDept(r.deptId);
        s += rline({r.date, r.type, d ? d->name : "-", dep ? dep->name : "-",
                    r.chief, r.diagnosis, r.advice}, ws) + "\n";
    }
    if (n == 0) s += "  （暂无医疗记录）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportPatientBills(const std::string& patientId) const {
    std::string s = "================ 费用账单 ================\n";
    auto bills = billsOfPatient(patientId);
    if (bills.empty()) s += "  （暂无账单）\n";
    double unpaid = 0, paid = 0;
    for (const auto* b : bills) {
        s += "账单 " + b->id + "   日期：" + b->date + "   状态：" + b->status +
             (b->status == BI_PAID ? "（缴费日期 " + b->payDate + "）" : "") + "\n";
        for (const auto& it : b->items)
            s += "    - " + it.category + "  " + it.desc + "  ￥" + fmtMoney(it.amount) + "\n";
        s += "    小计：￥" + fmtMoney(b->total()) + "\n";
        if (b->status == BI_UNPAID) unpaid += b->total(); else paid += b->total();
    }
    s += "------------------------------------------------\n";
    s += "已缴费合计：￥" + fmtMoney(paid) + "    待缴费合计：￥" + fmtMoney(unpaid) + "\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportPatientExams(const std::string& patientId) const {
    std::string s = "================ 检查与报告 ================\n";
    std::vector<int> ws = {9, 10, 20, 10, 30, 30};
    s += rline({"编号", "类别", "项目", "状态", "检查结果", "结论"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int n = 0;
    for (const auto& e : store.exams) {
        if (e.patientId != patientId) continue;
        ++n;
        const Report* rp = findReportByExam(e.id);
        s += rline({e.id, e.category, e.items, e.status,
                    rp ? rp->result : "-", rp ? rp->conclusion : "-"}, ws) + "\n";
    }
    if (n == 0) s += "  （暂无检查记录）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportPatientRx(const std::string& patientId) const {
    std::string s = "================ 处方与用药 ================\n";
    int n = 0;
    for (const auto& rx : store.prescriptions) {
        if (rx.patientId != patientId) continue;
        ++n;
        const Doctor* d = findDoctor(rx.doctorId);
        s += "处方 " + rx.id + "   日期：" + rx.date + "   状态：" + rx.status +
             "   开方医生：" + (d ? d->name : "-") + "\n";
        for (const auto& it : rx.items) {
            const Drug* g = findDrug(it.drugId);
            s += "    - " + (g ? g->genericName : it.drugId) +
                 (g && !g->brandName.empty() ? "（" + g->brandName + "）" : "") +
                 "  数量：" + std::to_string(it.qty) +
                 "  用法：" + it.usage +
                 "  ￥" + fmtMoney(g ? g->price * it.qty : 0) + "\n";
        }
        s += "    处方合计：￥" + fmtMoney(rxTotal(rx)) + "\n";
    }
    if (n == 0) s += "  （暂无处方记录）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportPatientHos(const std::string& patientId) const {
    std::string s = "================ 住院信息 ================\n";
    std::vector<int> ws = {9, 20, 10, 10, 10, 10, 12};
    s += rline({"住院号", "病房", "床位", "入院日期", "出院日期", "状态", "主治医生"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int n = 0;
    for (const auto& h : store.hospitalizations) {
        if (h.patientId != patientId) continue;
        ++n;
        const Ward* w = findWard(h.wardId);
        const Doctor* d = findDoctor(h.doctorId);
        s += rline({h.id, w ? w->name : "-", h.bedId, h.admitDate,
                    h.dischargeDate.empty() ? "-" : h.dischargeDate, h.status,
                    d ? d->name : "-"}, ws) + "\n";
        if (h.status == "住院中")
            s += "    已住院 " + std::to_string(activeDays(h)) + " 天，床位费 ￥" +
                 fmtMoney(w ? w->bedFee : 0) + "/天\n";
    }
    if (n == 0) s += "  （暂无住院记录）\n";
    s += "================================================\n";
    return s;
}

// ==================== 报表：医护视角（第7块） ====================

std::string Hospital::reportDailyOutpatient() const {
    std::string d = today();
    std::string s = "================ 今日门诊报表（" + d + "） ================\n";
    int total = 0, wait = 0, done = 0, cancel = 0;
    std::vector<int> ws = {9, 12, 10, 10, 8, 6, 8, 6};
    s += rline({"挂号单", "患者", "医生", "科室", "时段", "类型", "状态", "号序"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto& r : store.registrations) {
        if (r.date != d) continue;
        ++total;
        if (r.status == RS_WAIT) ++wait;
        else if (r.status == RS_DONE) ++done;
        else ++cancel;
        const Patient* p = findPatient(r.patientId);
        const Doctor* doc = findDoctor(r.doctorId);
        const Department* dep = findDept(r.deptId);
        s += rline({r.id, p ? p->name : "-", doc ? doc->name : "-", dep ? dep->name : "-",
                    r.slot, r.type, r.status, std::to_string(r.seq)}, ws) + "\n";
    }
    s += "------------------------------------------------\n";
    s += "合计：" + std::to_string(total) + "（待就诊 " + std::to_string(wait) +
         "，已就诊 " + std::to_string(done) + "，已取消 " + std::to_string(cancel) + "）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportDoctorWorkload() const {
    std::string s = "================ 医生接诊量统计 ================\n";
    std::vector<int> ws = {9, 10, 14, 10, 8, 8, 10};
    s += rline({"医生编号", "姓名", "科室", "职称", "挂号", "已诊", "待诊"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto& doc : store.doctors) {
        int total = 0, done = 0, wait = 0;
        for (const auto& r : store.registrations) {
            if (r.doctorId != doc.id) continue;
            ++total;
            if (r.status == RS_DONE) ++done;
            else if (r.status == RS_WAIT) ++wait;
        }
        const Department* dep = findDept(doc.deptId);
        s += rline({doc.id, doc.name, dep ? dep->name : "-", doc.title,
                    std::to_string(total), std::to_string(done), std::to_string(wait)}, ws) + "\n";
    }
    s += "================================================\n";
    return s;
}

std::string Hospital::reportBedOccupancy() const {
    std::string s = "================ 床位占用情况 ================\n";
    std::vector<int> ws = {9, 14, 8, 6, 6, 6, 6, 10};
    s += rline({"病房号", "病房名称", "类型", "床位数", "占用", "空闲", "其他", "占用率"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int tb = 0, to = 0;
    for (const auto& w : store.wards) {
        auto beds = bedsOfWard(w.id);
        int occ = 0, free = 0, other = 0;
        for (const auto* b : beds) {
            if (b->status == BS_OCCUPIED) ++occ;
            else if (b->status == BS_FREE) ++free;
            else ++other;
        }
        tb += (int)beds.size();
        to += occ;
        s += rline({w.id, w.name, w.type, std::to_string(beds.size()), std::to_string(occ),
                    std::to_string(free), std::to_string(other),
                    beds.empty() ? "-" : (std::to_string((int)std::round(100.0 * occ / beds.size())) + "%")}, ws) + "\n";
    }
    s += "------------------------------------------------\n";
    s += "全院床位：" + std::to_string(tb) + "，占用 " + std::to_string(to) + "，占用率 " +
         (tb ? std::to_string((int)std::round(100.0 * to / tb)) + "%" : "-") + "\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportLowStock() const {
    std::string s = "================ 库存预警（低于下限） ================\n";
    auto low = lowStockDrugs();
    if (low.empty()) { s += "  （当前无库存预警药品）\n"; }
    std::vector<int> ws = {9, 16, 16, 12, 10, 8, 8};
    s += rline({"编号", "通用名", "商品名", "分类", "库存", "下限", "状态"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto* d : low)
        s += rline({d->id, d->genericName, d->brandName, d->category, std::to_string(d->stock),
                    std::to_string(d->minStock), "预警"}, ws) + "\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportPendingRx() const {
    std::string s = "================ 待发药处方 ================\n";
    std::vector<int> ws = {9, 12, 10, 12, 30};
    s += rline({"处方号", "患者", "医生", "日期", "药品明细"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int n = 0;
    for (const auto& rx : store.prescriptions) {
        if (rx.status != RX_PENDING) continue;
        ++n;
        const Patient* p = findPatient(rx.patientId);
        const Doctor* doc = findDoctor(rx.doctorId);
        std::string items;
        for (const auto& it : rx.items) {
            const Drug* g = findDrug(it.drugId);
            if (!items.empty()) items += "；";
            items += (g ? g->genericName : it.drugId) + "x" + std::to_string(it.qty);
        }
        s += rline({rx.id, p ? p->name : "-", doc ? doc->name : "-", rx.date, items}, ws) + "\n";
    }
    if (n == 0) s += "  （暂无待发药处方）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportDoctorPatients(const std::string& doctorId) const {
    std::string s = "================ 我的患者 ================\n";
    s += "【今日挂号】\n";
    std::vector<int> ws = {9, 12, 8, 8, 10};
    s += rline({"挂号单", "患者", "时段", "状态", "号序"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto& r : store.registrations) {
        if (r.doctorId != doctorId || r.date != today()) continue;
        const Patient* p = findPatient(r.patientId);
        s += rline({r.id, p ? p->name : "-", r.slot, r.status, std::to_string(r.seq)}, ws) + "\n";
    }
    s += "【在院患者】\n";
    for (const auto& h : store.hospitalizations) {
        if (h.doctorId != doctorId || h.status != "住院中") continue;
        const Patient* p = findPatient(h.patientId);
        const Ward* w = findWard(h.wardId);
        s += "  " + h.id + "  " + (p ? p->name : "-") + "  " + (w ? w->name : "-") + "  " +
             h.bedId + "  入院 " + h.admitDate + "（第 " + std::to_string(activeDays(h)) + " 天）\n";
    }
    s += "================================================\n";
    return s;
}

// ==================== 报表：管理视角（第8块） ====================

std::string Hospital::reportDeptRevenue() const {
    std::string s = "================ 科室收入统计 ================\n";
    std::vector<int> ws = {9, 12, 12, 12, 12, 12, 14};
    s += rline({"科室号", "科室", "挂号收入", "检查收入", "药费收入", "床位收入", "合计"}, ws) + "\n";
    s += rsep(ws) + "\n";
    double all = 0;
    for (const auto& dep : store.departments) {
        double reg = 0, exam = 0, drug = 0, bed = 0;
        for (const auto& r : store.registrations)
            if (r.deptId == dep.id && r.status == RS_DONE) reg += r.fee;
        for (const auto& e : store.exams)
            if (e.deptId == dep.id && e.status == "已完成") exam += e.fee;
        for (const auto& rx : store.prescriptions)
            if (rx.deptId == dep.id && rx.status == RX_DISPENSED) drug += rxTotal(rx);
        for (const auto& h : store.hospitalizations) {
            if (h.status != "已出院") continue;
            const Ward* w = findWard(h.wardId);
            if (w && w->deptId == dep.id) bed += w->bedFee * std::max(1, _daysBetween(h.admitDate, h.dischargeDate));
        }
        double total = reg + exam + drug + bed;
        all += total;
        s += rline({dep.id, dep.name, fmtMoney(reg), fmtMoney(exam), fmtMoney(drug),
                    fmtMoney(bed), fmtMoney(total)}, ws) + "\n";
    }
    s += "------------------------------------------------\n";
    s += "全院合计收入：￥" + fmtMoney(all) + "\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportDrugFlow() const {
    std::string s = "================ 药品出入库台账统计 ================\n";
    std::vector<int> ws = {9, 16, 10, 10, 10, 10, 10};
    s += rline({"编号", "通用名", "入库量", "发药量", "退货量", "损耗量", "当前库存"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto& d : store.drugs) {
        int in = 0, disp = 0, ret = 0, loss = 0;
        for (const auto& sr : store.stockRecords) {
            if (sr.drugId != d.id) continue;
            if (sr.type == ST_IN) in += sr.qty;
            else if (sr.type == ST_DISPENSE) disp += sr.qty;
            else if (sr.type == ST_RETURN) ret += sr.qty;
            else if (sr.type == ST_LOSS) loss += sr.qty;
        }
        s += rline({d.id, d.genericName, std::to_string(in), std::to_string(disp),
                    std::to_string(ret), std::to_string(loss), std::to_string(d.stock)}, ws) + "\n";
    }
    s += "================================================\n";
    return s;
}

std::string Hospital::reportDrugSales() const {
    std::string s = "================ 药品销售排行（按发药金额） ================\n";
    std::vector<std::pair<std::string, double>> sales;
    std::map<std::string, double> qtyMap;
    for (const auto& rx : store.prescriptions) {
        if (rx.status != RX_DISPENSED) continue;
        for (const auto& it : rx.items) {
            const Drug* g = findDrug(it.drugId);
            if (!g) continue;
            qtyMap[it.drugId] += it.qty;
        }
    }
    for (const auto& kv : qtyMap) {
        const Drug* g = findDrug(kv.first);
        if (g) sales.push_back({kv.first, g->price * kv.second});
    }
    std::sort(sales.begin(), sales.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    std::vector<int> ws = {9, 16, 16, 12, 10, 12};
    s += rline({"编号", "通用名", "商品名", "发药数量", "单价", "销售金额"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto& kv : sales) {
        const Drug* g = findDrug(kv.first);
        s += rline({g->id, g->genericName, g->brandName, std::to_string((int)qtyMap[kv.first]),
                    fmtMoney(g->price), fmtMoney(kv.second)}, ws) + "\n";
    }
    if (sales.empty()) s += "  （暂无销售数据）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportBedTurnover() const {
    std::string s = "================ 床位周转统计 ================\n";
    std::vector<int> ws = {9, 14, 8, 8, 8, 14};
    s += rline({"病房号", "病房名称", "床位数", "出院人次", "占用床位", "平均住院天数"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto& w : store.wards) {
        int discharged = 0, days = 0, occ = 0;
        for (const auto& h : store.hospitalizations) {
            if (h.wardId != w.id) continue;
            if (h.status == "已出院") {
                ++discharged;
                days += std::max(1, _daysBetween(h.admitDate, h.dischargeDate));
            } else {
                ++occ;
                days += std::max(1, _daysBetween(h.admitDate, today()));
            }
        }
        auto beds = bedsOfWard(w.id);
        s += rline({w.id, w.name, std::to_string(beds.size()), std::to_string(discharged),
                    std::to_string(occ),
                    discharged ? fmtMoney((double)days / discharged) : "-"}, ws) + "\n";
    }
    s += "================================================\n";
    return s;
}

std::string Hospital::reportPatientFlow() const {
    std::string s = "================ 患者流量（近 7 天） ================\n";
    std::vector<int> ws = {12, 10, 10, 10};
    s += rline({"日期", "挂号数", "就诊数", "取消数"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (int off = 6; off >= 0; --off) {
        std::time_t t = std::time(nullptr) - off * 86400;
        std::tm tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        std::string d = oss.str();
        int total = 0, done = 0, cancel = 0;
        for (const auto& r : store.registrations) {
            if (r.date != d) continue;
            ++total;
            if (r.status == RS_DONE) ++done;
            else if (r.status == RS_CANCEL) ++cancel;
        }
        s += rline({d, std::to_string(total), std::to_string(done), std::to_string(cancel)}, ws) + "\n";
    }
    s += "================================================\n";
    return s;
}

std::string Hospital::reportAll() const {
    std::string s = "================ 全院运营总览 ================\n";
    int outpatients = 0, inpatients = 0;
    for (const auto& p : store.patients)
        if (p.status == PS_INPATIENT) ++inpatients; else ++outpatients;
    double paidTotal = 0, unpaidTotal = 0;
    for (const auto& b : store.bills)
        if (b.status == BI_PAID) paidTotal += b.total(); else unpaidTotal += b.total();
    int regTotal = (int)store.registrations.size();
    int rxPending = 0;
    for (const auto& rx : store.prescriptions) if (rx.status == RX_PENDING) ++rxPending;
    s += "  患者总数：" + std::to_string(store.patients.size()) + "（门诊 " +
         std::to_string(outpatients) + "，住院 " + std::to_string(inpatients) + "）\n";
    s += "  医生总数：" + std::to_string(store.doctors.size()) + "    科室数：" +
         std::to_string(store.departments.size()) + "\n";
    s += "  累计挂号单：" + std::to_string(regTotal) + "    检查单：" + std::to_string(store.exams.size()) +
         "    处方：" + std::to_string(store.prescriptions.size()) + "\n";
    s += "  在院患者：" + std::to_string(inpatients) + "    待发药处方：" + std::to_string(rxPending) + "\n";
    s += "  药品库存预警：" + std::to_string(lowStockDrugs().size()) + " 种\n";
    s += "  已缴费金额：￥" + fmtMoney(paidTotal) + "    待缴费金额：￥" + fmtMoney(unpaidTotal) + "\n";
    s += "================================================\n";
    return s;
}

// ==================== 基础信息查询与工具（第9块） ====================

std::string Hospital::reportByPatient(const std::string& keyword) const {
    auto list = searchPatients(keyword);
    std::string s = "================ 患者查询（关键词：" + keyword + "） ================\n";
    if (list.empty()) { s += "  （未找到匹配患者）\n"; s += "================================================\n"; return s; }
    std::vector<int> ws = {9, 14, 8, 8, 14, 8, 10};
    s += rline({"患者编号", "姓名", "性别", "年龄", "身份证号", "状态", "挂号次数"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto* p : list) {
        int regs = 0;
        for (const auto& r : store.registrations) if (r.patientId == p->id) ++regs;
        s += rline({p->id, p->name, p->gender, std::to_string(p->age), p->idCard,
                    p->status, std::to_string(regs)}, ws) + "\n";
    }
    s += "------------------------------------------------\n";
    s += "共 " + std::to_string(list.size()) + " 名患者（同名患者请按患者编号区分）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportByDoctor(const std::string& keyword) const {
    std::string s = "================ 医生查询（关键词：" + keyword + "） ================\n";
    std::vector<int> ws = {9, 10, 14, 12, 12, 10, 10};
    s += rline({"医生编号", "姓名", "科室", "职称", "专长", "接诊量", "在院患者"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int shown = 0;
    for (const auto& doc : store.doctors) {
        if (!containsIgnoreCase(doc.name, keyword) && !containsIgnoreCase(doc.id, keyword) &&
            !containsIgnoreCase(doc.title, keyword) && !containsIgnoreCase(doc.specialty, keyword))
            continue;
        ++shown;
        const Department* dep = findDept(doc.deptId);
        int done = 0, inHos = 0;
        for (const auto& r : store.registrations) if (r.doctorId == doc.id && r.status == RS_DONE) ++done;
        for (const auto& h : store.hospitalizations)
            if (h.doctorId == doc.id && h.status == "住院中") ++inHos;
        s += rline({doc.id, doc.name, dep ? dep->name : "-", doc.title, doc.specialty,
                    std::to_string(done), std::to_string(inHos)}, ws) + "\n";
    }
    if (!shown) s += "  （未找到匹配医生）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportByDept(const std::string& keyword) const {
    std::string s = "================ 科室查询（关键词：" + keyword + "） ================\n";
    std::vector<int> ws = {9, 12, 14, 8, 8, 8, 10};
    s += rline({"科室号", "科室名称", "位置", "医生数", "常用药数", "今日挂号", "累计挂号"}, ws) + "\n";
    s += rsep(ws) + "\n";
    int shown = 0;
    for (const auto& dep : store.departments) {
        if (!containsIgnoreCase(dep.name, keyword) && !containsIgnoreCase(dep.id, keyword) &&
            !containsIgnoreCase(dep.location, keyword))
            continue;
        ++shown;
        int doctors = 0, drugs = 0, todayRegs = 0, totalRegs = 0;
        for (const auto& doc : store.doctors) if (doc.deptId == dep.id) ++doctors;
        drugs = (int)dep.drugIds.size();
        for (const auto& r : store.registrations) {
            if (r.deptId != dep.id) continue;
            ++totalRegs;
            if (r.date == today()) ++todayRegs;
        }
        s += rline({dep.id, dep.name, dep.location, std::to_string(doctors), std::to_string(drugs),
                    std::to_string(todayRegs), std::to_string(totalRegs)}, ws) + "\n";
    }
    if (!shown) s += "  （未找到匹配科室）\n";
    s += "================================================\n";
    return s;
}

std::string Hospital::reportByDrug(const std::string& keyword) const {
    auto list = searchDrugs(keyword);
    std::string s = "================ 药品查询（关键词：" + keyword + "） ================\n";
    if (list.empty()) { s += "  （未找到匹配药品，支持按通用名/商品名/别名查询）\n"; s += "================================================\n"; return s; }
    std::vector<int> ws = {9, 16, 16, 16, 10, 10, 8, 8, 8};
    s += rline({"编号", "通用名", "商品名", "别名", "分类", "规格", "单价", "库存", "下限"}, ws) + "\n";
    s += rsep(ws) + "\n";
    for (const auto* d : list) {
        s += rline({d->id, d->genericName, d->brandName, d->alias, d->category, d->spec,
                    fmtMoney(d->price), std::to_string(d->stock), std::to_string(d->minStock)}, ws) + "\n";
    }
    s += "------------------------------------------------\n";
    s += "共 " + std::to_string(list.size()) + " 种药品（含库存预警 "
         + std::to_string(std::count_if(list.begin(), list.end(),
             [](const Drug* d) { return d->stock <= d->minStock; })) + " 种）\n";
    s += "================================================\n";
    return s;
}

// ==================== 工具 ====================

int Hospital::_daysBetween(const std::string& a, const std::string& b) const {
    auto parse = [](const std::string& d, std::tm& tm) -> bool {
        std::istringstream iss(d);
        char c1, c2;
        iss >> tm.tm_year >> c1 >> tm.tm_mon >> c2 >> tm.tm_mday;
        if (iss.fail() || c1 != '-' || c2 != '-') return false;
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        tm.tm_hour = 12;
        return true;
    };
    std::tm ta{}, tb{};
    if (!parse(a, ta) || !parse(b, tb)) return 0;
    std::time_t ta_t = std::mktime(&ta);
    std::time_t tb_t = std::mktime(&tb);
    double diff = std::difftime(tb_t, ta_t) / 86400.0;
    return (int)std::round(diff);
}

} // namespace hm
