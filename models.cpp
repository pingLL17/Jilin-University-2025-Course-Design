// ============================================================
// 医疗管理系统 - 实体类序列化实现
// ============================================================
#include "models.h"

namespace hm {

// ---------- 通用人员字段 ----------
static std::string personToLine(const Person& p) {
    return p.id + "|" + p.name + "|" + p.gender + "|" + std::to_string(p.age) +
           "|" + p.idCard + "|" + p.phone + "|" + p.username + "|" + p.password + "|" + p.role;
}

static bool personFromLine(const std::vector<std::string>& v, Person& p) {
    if (v.size() < 9) return false;
    p.id = v[0];
    p.name = v[1];
    p.gender = v[2];
    p.age = toInt(v[3]);
    p.idCard = v[4];
    p.phone = v[5];
    p.username = v[6];
    p.password = v[7];
    p.role = v[8];
    return !p.id.empty() && !p.name.empty();
}

std::string Patient::toLine() const {
    return personToLine(*this) + "|" + bloodType + "|" + allergy + "|" + address + "|" + regDate + "|" + status;
}
bool Patient::fromLine(const std::string& line, Patient& p) {
    auto v = split(line, '|');
    if (v.size() < 14 || !personFromLine(v, p)) return false;
    p.bloodType = v[9];
    p.allergy = v[10];
    p.address = v[11];
    p.regDate = v[12];
    p.status = v[13];
    return true;
}

std::string Doctor::toLine() const {
    return personToLine(*this) + "|" + deptId + "|" + title + "|" + specialty;
}
bool Doctor::fromLine(const std::string& line, Doctor& d) {
    auto v = split(line, '|');
    if (v.size() < 12 || !personFromLine(v, d)) return false;
    d.deptId = v[9];
    d.title = v[10];
    d.specialty = v[11];
    return true;
}

std::string Nurse::toLine() const {
    return personToLine(*this) + "|" + deptId;
}
bool Nurse::fromLine(const std::string& line, Nurse& n) {
    auto v = split(line, '|');
    if (v.size() < 10 || !personFromLine(v, n)) return false;
    n.deptId = v[9];
    return true;
}

std::string Pharmacist::toLine() const { return personToLine(*this); }
bool Pharmacist::fromLine(const std::string& line, Pharmacist& p) {
    auto v = split(line, '|');
    return personFromLine(v, p);
}

std::string Admin::toLine() const { return personToLine(*this); }
bool Admin::fromLine(const std::string& line, Admin& a) {
    auto v = split(line, '|');
    return personFromLine(v, a);
}

std::string Department::toLine() const {
    return id + "|" + name + "|" + location + "|" + desc + "|" + wardType + "|" + join(drugIds, ';');
}
bool Department::fromLine(const std::string& line, Department& d) {
    auto v = split(line, '|');
    if (v.size() < 5) return false;
    d.id = v[0];
    d.name = v[1];
    d.location = v[2];
    d.desc = v[3];
    d.wardType = v[4];
    d.drugIds = v.size() > 5 && !v[5].empty() ? split(v[5], ';') : std::vector<std::string>{};
    return !d.id.empty();
}

std::string Registration::toLine() const {
    return id + "|" + patientId + "|" + doctorId + "|" + deptId + "|" + date + "|" + slot +
           "|" + type + "|" + status + "|" + std::to_string(seq) + "|" + fmtMoney(fee);
}
bool Registration::fromLine(const std::string& line, Registration& r) {
    auto v = split(line, '|');
    if (v.size() < 10) return false;
    r.id = v[0];
    r.patientId = v[1];
    r.doctorId = v[2];
    r.deptId = v[3];
    r.date = v[4];
    r.slot = v[5];
    r.type = v[6];
    r.status = v[7];
    r.seq = toInt(v[8]);
    r.fee = toDouble(v[9]);
    return !r.id.empty();
}

std::string MedicalRecord::toLine() const {
    return id + "|" + patientId + "|" + doctorId + "|" + deptId + "|" + type + "|" + date + "|" + time +
           "|" + chief + "|" + diagnosis + "|" + advice + "|" + linkedId;
}
bool MedicalRecord::fromLine(const std::string& line, MedicalRecord& r) {
    auto v = split(line, '|');
    if (v.size() < 11) return false;
    r.id = v[0];
    r.patientId = v[1];
    r.doctorId = v[2];
    r.deptId = v[3];
    r.type = v[4];
    r.date = v[5];
    r.time = v[6];
    r.chief = v[7];
    r.diagnosis = v[8];
    r.advice = v[9];
    r.linkedId = v[10];
    return !r.id.empty();
}

std::string Examination::toLine() const {
    return id + "|" + patientId + "|" + doctorId + "|" + deptId + "|" + category + "|" + items +
           "|" + date + "|" + status + "|" + fmtMoney(fee);
}
bool Examination::fromLine(const std::string& line, Examination& e) {
    auto v = split(line, '|');
    if (v.size() < 9) return false;
    e.id = v[0];
    e.patientId = v[1];
    e.doctorId = v[2];
    e.deptId = v[3];
    e.category = v[4];
    e.items = v[5];
    e.date = v[6];
    e.status = v[7];
    e.fee = toDouble(v[8]);
    return !e.id.empty();
}

std::string Report::toLine() const {
    return id + "|" + examId + "|" + result + "|" + conclusion + "|" + date + "|" + doctorId;
}
bool Report::fromLine(const std::string& line, Report& r) {
    auto v = split(line, '|');
    if (v.size() < 6) return false;
    r.id = v[0];
    r.examId = v[1];
    r.result = v[2];
    r.conclusion = v[3];
    r.date = v[4];
    r.doctorId = v[5];
    return !r.id.empty();
}

std::string Prescription::toLine() const {
    std::vector<std::string> fs;
    for (const auto& it : items) fs.push_back(it.toField());
    return id + "|" + patientId + "|" + doctorId + "|" + deptId + "|" + date + "|" + status + "|" + join(fs, ';');
}
bool Prescription::fromLine(const std::string& line, Prescription& rx) {
    auto v = split(line, '|');
    if (v.size() < 6) return false;
    rx.id = v[0];
    rx.patientId = v[1];
    rx.doctorId = v[2];
    rx.deptId = v[3];
    rx.date = v[4];
    rx.status = v[5];
    rx.items.clear();
    if (v.size() > 6 && !v[6].empty()) {
        for (const auto& f : split(v[6], ';')) {
            PrescriptionItem it;
            if (PrescriptionItem::fromField(f, it)) rx.items.push_back(it);
        }
    }
    return !rx.id.empty();
}

std::string Ward::toLine() const {
    return id + "|" + name + "|" + type + "|" + deptId + "|" + desc + "|" + fmtMoney(bedFee);
}
bool Ward::fromLine(const std::string& line, Ward& w) {
    auto v = split(line, '|');
    if (v.size() < 6) return false;
    w.id = v[0];
    w.name = v[1];
    w.type = v[2];
    w.deptId = v[3];
    w.desc = v[4];
    w.bedFee = toDouble(v[5]);
    return !w.id.empty();
}

std::string Bed::toLine() const {
    return id + "|" + wardId + "|" + status + "|" + patientId;
}
bool Bed::fromLine(const std::string& line, Bed& b) {
    auto v = split(line, '|');
    if (v.size() < 4) return false;
    b.id = v[0];
    b.wardId = v[1];
    b.status = v[2];
    b.patientId = v[3];
    return !b.id.empty();
}

std::string Hospitalization::toLine() const {
    return id + "|" + patientId + "|" + bedId + "|" + wardId + "|" + doctorId + "|" + admitDate +
           "|" + dischargeDate + "|" + status + "|" + remark;
}
bool Hospitalization::fromLine(const std::string& line, Hospitalization& h) {
    auto v = split(line, '|');
    if (v.size() < 9) return false;
    h.id = v[0];
    h.patientId = v[1];
    h.bedId = v[2];
    h.wardId = v[3];
    h.doctorId = v[4];
    h.admitDate = v[5];
    h.dischargeDate = v[6];
    h.status = v[7];
    h.remark = v[8];
    return !h.id.empty();
}

std::string NursingRecord::toLine() const {
    return id + "|" + patientId + "|" + bedId + "|" + nurseId + "|" + date + "|" + time + "|" + content;
}
bool NursingRecord::fromLine(const std::string& line, NursingRecord& n) {
    auto v = split(line, '|');
    if (v.size() < 7) return false;
    n.id = v[0];
    n.patientId = v[1];
    n.bedId = v[2];
    n.nurseId = v[3];
    n.date = v[4];
    n.time = v[5];
    n.content = v[6];
    return !n.id.empty();
}

std::string Drug::toLine() const {
    return id + "|" + genericName + "|" + brandName + "|" + alias + "|" + category + "|" + spec + "|" + unit +
           "|" + deptId + "|" + manufacturer + "|" + expiry + "|" + fmtMoney(price) + "|" +
           std::to_string(stock) + "|" + std::to_string(minStock);
}
bool Drug::fromLine(const std::string& line, Drug& d) {
    auto v = split(line, '|');
    if (v.size() < 13) return false;
    d.id = v[0];
    d.genericName = v[1];
    d.brandName = v[2];
    d.alias = v[3];
    d.category = v[4];
    d.spec = v[5];
    d.unit = v[6];
    d.deptId = v[7];
    d.manufacturer = v[8];
    d.expiry = v[9];
    d.price = toDouble(v[10]);
    d.stock = toInt(v[11]);
    d.minStock = toInt(v[12]);
    return !d.id.empty();
}

std::string StockRecord::toLine() const {
    return id + "|" + drugId + "|" + type + "|" + date + "|" + operatorName + "|" + remark + "|" + refId +
           "|" + std::to_string(qty);
}
bool StockRecord::fromLine(const std::string& line, StockRecord& s) {
    auto v = split(line, '|');
    if (v.size() < 8) return false;
    s.id = v[0];
    s.drugId = v[1];
    s.type = v[2];
    s.date = v[3];
    s.operatorName = v[4];
    s.remark = v[5];
    s.refId = v[6];
    s.qty = toInt(v[7]);
    return !s.id.empty();
}

std::string Bill::toLine() const {
    std::vector<std::string> fs;
    for (const auto& it : items) fs.push_back(it.toField());
    return id + "|" + patientId + "|" + date + "|" + status + "|" + payDate + "|" + join(fs, ';');
}
bool Bill::fromLine(const std::string& line, Bill& b) {
    auto v = split(line, '|');
    if (v.size() < 5) return false;
    b.id = v[0];
    b.patientId = v[1];
    b.date = v[2];
    b.status = v[3];
    b.payDate = v[4];
    b.items.clear();
    if (v.size() > 5 && !v[5].empty()) {
        for (const auto& f : split(v[5], ';')) {
            BillItem it;
            if (BillItem::fromField(f, it)) b.items.push_back(it);
        }
    }
    return !b.id.empty();
}

} // namespace hm
