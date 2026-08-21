// ============================================================
// 医疗管理系统 - 控制台界面（第1块：主流程/登录/注册/通用选择）
// ============================================================
#include "ui.h"
#include <iostream>

namespace hm {

namespace {

struct Session {
    std::string id;   // 当前登录用户编号
    std::string role; // 角色
};

void banner() {
    std::cout << "\n==============================================\n";
    std::cout << "        医疗管理系统\n";
    std::cout << "==============================================\n";
}

// ---------- 通用选择辅助 ----------

void printDepts(const Hospital& h) {
    int i = 1;
    for (const auto& d : h.store.departments)
        std::cout << "  " << i++ << ". " << d.id << " " << d.name << "（" << d.location << "）\n";
}

const Department* pickDept(Hospital& h) {
    printDepts(h);
    int c = inputInt("  请选择科室（0=返回）：", 0, (int)h.store.departments.size());
    if (c == 0) return nullptr;
    return &h.store.departments[c - 1];
}

const Doctor* pickDoctor(Hospital& h, const std::string& deptId) {
    auto docs = h.doctorsOfDept(deptId);
    if (docs.empty()) { std::cout << "  该科室暂无医生。\n"; return nullptr; }
    for (size_t i = 0; i < docs.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << docs[i]->id << " " << docs[i]->name
                  << "（" << docs[i]->title << "） 专长：" << docs[i]->specialty << "\n";
    int c = inputInt("  请选择医生（0=返回）：", 0, (int)docs.size());
    if (c == 0) return nullptr;
    return docs[c - 1];
}

const Patient* pickPatient(Hospital& h) {
    std::string kw = inputLine("  输入患者姓名/编号/身份证号（0=返回）：", MAX_IDCARD_LEN + 4);
    if (kw == "0") return nullptr;
    auto list = h.searchPatients(kw);
    if (list.empty()) { std::cout << "  未找到匹配患者。\n"; return nullptr; }
    if (list.size() == 1) return list[0];
    std::cout << "  找到 " << list.size() << " 名患者（含同名患者，请按编号区分）：\n";
    for (size_t i = 0; i < list.size(); ++i) {
        const Patient* p = list[i];
        std::string tail = p->idCard.size() >= 4 ? p->idCard.substr(p->idCard.size() - 4) : p->idCard;
        std::cout << "  " << (i + 1) << ". " << p->id << " " << p->name << " 性别:" << p->gender
                  << " 年龄:" << p->age << " 状态:" << p->status << " 身份证尾号:" << tail << "\n";
    }
    int c = inputInt("  请选择（0=取消）：", 0, (int)list.size());
    if (c == 0) return nullptr;
    return list[c - 1];
}

const Drug* pickDrug(Hospital& h) {
    std::string kw = inputLine("  输入药品通用名/商品名/别名（0=返回）：", MAX_DRUG_NAME_LEN + 4);
    if (kw == "0") return nullptr;
    auto list = h.searchDrugs(kw);
    if (list.empty()) { std::cout << "  未找到匹配药品。\n"; return nullptr; }
    std::cout << "  找到 " << list.size() << " 种药品：\n";
    for (size_t i = 0; i < list.size(); ++i) {
        const Drug* d = list[i];
        std::cout << "  " << (i + 1) << ". " << d->id << " " << d->genericName
                  << "（" << d->brandName << "） ￥" << fmtMoney(d->price)
                  << " 库存:" << d->stock << "\n";
    }
    int c = inputInt("  请选择（0=返回）：", 0, (int)list.size());
    if (c == 0) return nullptr;
    return list[c - 1];
}

const Ward* pickWard(Hospital& h, bool needFreeBed) {
    (void)needFreeBed;
    int i = 1;
    for (const auto& w : h.store.wards) {
        auto beds = h.bedsOfWard(w.id);
        int free = 0;
        for (const auto* b : beds) if (b->status == BS_FREE) ++free;
        std::cout << "  " << i++ << ". " << w.id << " " << w.name << "（" << w.type
                  << "） 床位:" << beds.size() << " 空闲:" << free
                  << " ￥" << fmtMoney(w.bedFee) << "/天"
                  << (w.deptId.empty() ? "" : " [关联科室]") << "\n";
    }
    int c = inputInt("  请选择病房（0=返回）：", 0, (int)h.store.wards.size());
    if (c == 0) return nullptr;
    return &h.store.wards[c - 1];
}

std::string pickSlot() {
    std::cout << "  1. 上午\n  2. 下午\n";
    int c = inputInt("  请选择就诊时段：", 1, 2);
    return c == 1 ? "上午" : "下午";
}

void printPerson(const Person& p) {
    std::cout << "  当前登录：" << p.name << "（" << p.role << "） 编号：" << p.id << "\n";
}

// ---------- 登录与注册 ----------

void registerScreen(Hospital& h) {
    std::cout << "\n---------- 患者自助注册建档 ----------\n";
    Patient p;
    p.name = inputLine("  姓名（必填，可与其他患者重名）：", MAX_NAME_LEN);
    std::cout << "  性别：1.男 2.女\n";
    p.gender = inputInt("  请选择：", 1, 2) == 1 ? "男" : "女";
    p.age = inputInt("  年龄：", 0, 120);
    p.idCard = inputIdCard("  身份证号（唯一校验，15或18位）：");
    p.phone = inputPhone("  联系电话（11位手机号）：");
    p.address = inputLine("  家庭住址（可空，直接回车跳过）：", MAX_ADDR_LEN, true);
    p.bloodType = inputLine("  血型（A/B/O/AB/未知，可回车跳过）：", 8, true);
    p.allergy = inputLine("  过敏史（无/具体过敏原，可回车跳过）：", MAX_TEXT_LEN, true);
    p.username = inputLine("  设置登录账号（字母数字，4~20位）：", MAX_USERNAME_LEN);
    p.password = inputLine("  设置登录密码（4~20位）：", MAX_PASSWORD_LEN);
    if (p.username.size() < 4) { std::cout << "  账号长度至少 4 位。\n"; return; }
    if (p.password.size() < 4) { std::cout << "  密码长度至少 4 位。\n"; return; }
    bool alnum = !p.username.empty();
    for (char ch : p.username)
        if (!std::isalnum((unsigned char)ch)) { alnum = false; break; }
    if (!alnum) { std::cout << "  账号只能包含字母和数字。\n"; return; }
    std::string err;
    if (h.patientRegister(p, err)) {
        std::cout << "  注册成功！您的患者编号：" << p.id << "，请使用账号 " << p.username << " 登录。\n";
    } else {
        std::cout << "  注册失败：" << err << "\n";
    }
}

void loginScreen(Hospital& h, Session& sess) {
    std::cout << "\n---------- 用户登录 ----------\n";
    std::cout << "  （演示账号：患者/医生/护士/药剂师编号如 P00001、D00001，管理员 admin，密码均 123456）\n";
    std::string un = inputLine("  账号：", MAX_USERNAME_LEN);
    std::string pw = inputLine("  密码：", MAX_PASSWORD_LEN);
    std::string err;
    const Person* p = h.login(un, pw, err);
    if (!p) {
        std::cout << "  登录失败：" << err << "\n";
        return;
    }
    sess.id = p->id;
    sess.role = p->role;
    std::cout << "  登录成功！欢迎 " << p->name << "（" << p->role << "）\n";
}

} // namespace

// 角色菜单（在后续块中定义，匿名命名空间保证内部链接）
namespace {
void patientMenu(Hospital& h, const Session& s);
void doctorMenu(Hospital& h, const Session& s);
void nurseMenu(Hospital& h, const Session& s);
void pharmacistMenu(Hospital& h, const Session& s);
void adminMenu(Hospital& h, const Session& s);
} // namespace
void runApp(Hospital& h) {
    Session sess;
    while (true) {
        banner();
        std::cout << "  1. 登录\n  2. 患者自助注册\n  0. 退出系统\n";
        int c = inputInt("  请选择：", 0, 2);
        if (c == 0) { std::cout << "  已退出系统，再见！\n"; break; }
        if (c == 2) { registerScreen(h); pause(); continue; }
        sess = Session{};
        loginScreen(h, sess);
        if (sess.id.empty()) { pause(); continue; }
        if (sess.role == ROLE_PATIENT) patientMenu(h, sess);
        else if (sess.role == ROLE_DOCTOR) doctorMenu(h, sess);
        else if (sess.role == ROLE_NURSE) nurseMenu(h, sess);
        else if (sess.role == ROLE_PHARMACIST) pharmacistMenu(h, sess);
        else if (sess.role == ROLE_ADMIN) adminMenu(h, sess);
        else { std::cout << "  未知角色。\n"; }
    }
}

// ============================================================
// 患者菜单（第2块）
// ============================================================
namespace {

void patientProfile(Hospital& h, const Session& s) {
    const Patient* p = h.findPatient(s.id);
    if (!p) return;
    std::cout << h.reportPatientInfo(s.id);
    std::cout << "---------- 修改个人信息（直接回车保持原值） ----------\n";
    Patient np = *p;
    std::string phone = inputPhone("  联系电话（" + p->phone + "，直接回车保持原值）：", true);
    if (!phone.empty()) np.phone = phone;
    std::string addr = inputLine("  家庭住址（" + p->address + "）：", MAX_ADDR_LEN, true);
    if (!addr.empty()) np.address = addr;
    std::string blood = inputLine("  血型（" + p->bloodType + "）：", 8, true);
    if (!blood.empty()) np.bloodType = blood;
    std::string allergy = inputLine("  过敏史（" + p->allergy + "）：", MAX_TEXT_LEN, true);
    if (!allergy.empty()) np.allergy = allergy;
    std::string err;
    if (h.updatePatientProfile(np, err)) std::cout << "  个人信息已更新。\n";
    else std::cout << "  更新失败：" << err << "\n";
}

void patientBook(Hospital& h, const Session& s, bool walkIn) {
    const Department* dep = pickDept(h);
    if (!dep) return;
    const Doctor* doc = pickDoctor(h, dep->id);
    if (!doc) return;
    std::string date = today();
    std::string type = walkIn ? "现场" : "预约";
    if (!walkIn) {
        std::cout << "  输入就诊日期（如 2026-08-22，当天为 " << today() << "）：\n";
        date = inputLine("  日期：", 10);
        if (!isValidDate(date)) { std::cout << "  日期无效（须为真实日期，如 2026-08-22）。\n"; return; }
        if (date < today()) { std::cout << "  不能预约过去的日期。\n"; return; }
    }
    std::string slot = pickSlot();
    std::cout << "  确认：" << type << "挂号 " << dep->name << " " << doc->name
              << " " << date << " " << slot << "？\n";
    if (inputYesNo("  确认") != "y") return;
    std::string err;
    std::string regId = h.createRegistration(s.id, doc->id, date, slot, type, err);
    if (regId.empty()) std::cout << "  挂号失败：" << err << "\n";
    else std::cout << "  挂号成功！挂号单号：" << regId << "，请按时就诊。\n";
}

void patientMyRegs(Hospital& h, const Session& s) {
    std::cout << "================ 我的挂号记录 ================\n";
    int i = 1;
    std::vector<std::string> ids;
    for (const auto& r : h.store.registrations) {
        if (r.patientId != s.id) continue;
        const Doctor* doc = h.findDoctor(r.doctorId);
        const Department* dep = h.findDept(r.deptId);
        std::cout << "  " << i++ << ". " << r.id << " " << r.date << " " << r.slot
                  << " " << (dep ? dep->name : "-") << " " << (doc ? doc->name : "-")
                  << " 号序:" << r.seq << " " << r.type << " " << r.status << " ￥" << fmtMoney(r.fee) << "\n";
        ids.push_back(r.id);
    }
    if (ids.empty()) { std::cout << "  （暂无挂号记录）\n"; return; }
    if (inputYesNo("  是否取消某个挂号？") != "y") return;
    int c = inputInt("  请输入挂号序号（1~" + std::to_string(ids.size()) + "）：", 1, (int)ids.size());
    std::string err;
    if (h.cancelRegistration(ids[c - 1], err)) std::cout << "  已取消挂号。\n";
    else std::cout << "  取消失败：" << err << "\n";
}

void patientPay(Hospital& h, const Session& s) {
    auto bills = h.billsOfPatient(s.id);
    bool any = false;
    for (const auto* b : bills) {
        if (b->status != BI_UNPAID) continue;
        any = true;
        std::cout << "  " << b->id << " 日期:" << b->date << " 金额:￥" << fmtMoney(b->total());
        std::cout << " 明细:";
        for (const auto& it : b->items) std::cout << it.category << " ";
        std::cout << "\n";
    }
    if (!any) { std::cout << "  当前没有待缴费账单。\n"; return; }
    std::string bid = inputLine("  输入要缴费的账单号（0=返回）：", 20);
    if (bid == "0") return;
    std::string err;
    if (h.payBill(bid, s.id, err)) std::cout << "  缴费成功！\n";
    else std::cout << "  缴费失败：" << err << "\n";
}

void patientMenu(Hospital& h, const Session& s) {
    while (true) {
        clearScreen();
        banner();
        const Patient* p = h.findPatient(s.id);
        std::cout << "---------- 患者服务菜单 ----------\n";
        if (p) printPerson(*p);
        std::cout << "  1. 查看/修改个人信息\n";
        std::cout << "  2. 预约挂号\n";
        std::cout << "  3. 现场挂号\n";
        std::cout << "  4. 我的挂号记录（可取消）\n";
        std::cout << "  5. 就诊与医疗记录\n";
        std::cout << "  6. 检查与报告\n";
        std::cout << "  7. 处方与用药\n";
        std::cout << "  8. 住院信息\n";
        std::cout << "  9. 费用账单（缴费）\n";
        std::cout << "  10. 修改密码\n";
        std::cout << "  0. 退出登录\n";
        int c = inputInt("  请选择：", 0, 10);
        switch (c) {
            case 1: patientProfile(h, s); break;
            case 2: patientBook(h, s, false); break;
            case 3: patientBook(h, s, true); break;
            case 4: patientMyRegs(h, s); break;
            case 5: std::cout << h.reportPatientRecords(s.id); break;
            case 6: std::cout << h.reportPatientExams(s.id); break;
            case 7: std::cout << h.reportPatientRx(s.id); break;
            case 8: std::cout << h.reportPatientHos(s.id); break;
            case 9: patientPay(h, s); break;
            case 10: {
                std::string oldPw = inputLine("  原密码：", MAX_PASSWORD_LEN);
                std::string newPw = inputLine("  新密码（4~20位）：", MAX_PASSWORD_LEN);
                std::string err;
                if (h.changePassword(s.id, s.role, oldPw, newPw, err)) std::cout << "  密码修改成功。\n";
                else std::cout << "  修改失败：" << err << "\n";
                break;
            }
            case 0: return;
        }
        pause();
    }
}

} // namespace

// ============================================================
// 医生菜单（第3块）
// ============================================================
namespace {

void doctorConsult(Hospital& h, const Session& s) {
    std::cout << "================ 待就诊挂号（今日） ================\n";
    std::vector<std::string> regIds;
    int i = 1;
    for (const auto& r : h.store.registrations) {
        if (r.doctorId != s.id || r.date != today() || r.status != RS_WAIT) continue;
        const Patient* p = h.findPatient(r.patientId);
        std::cout << "  " << i++ << ". " << r.id << " " << (p ? p->name : "-") << " "
                  << r.slot << " 号序:" << r.seq << "\n";
        regIds.push_back(r.id);
    }
    if (regIds.empty()) { std::cout << "  （今日暂无待就诊患者）\n"; return; }
    int c = inputInt("  选择接诊的挂号单（0=返回）：", 0, (int)regIds.size());
    if (c == 0) return;
    std::string regId = regIds[c - 1];
    std::string chief = inputLine("  主诉：", MAX_TEXT_LEN);
    std::string diag = inputLine("  诊断：", MAX_TEXT_LEN);
    std::string advice = inputLine("  医嘱：", MAX_TEXT_LEN);
    std::string err;
    if (h.consult(regId, chief, diag, advice, err)) std::cout << "  看诊完成，病历已记录。\n";
    else std::cout << "  看诊失败：" << err << "\n";
}

void doctorCreateExam(Hospital& h, const Session& s) {
    const Patient* p = pickPatient(h);
    if (!p) return;
    std::cout << "  常见检查类别：\n  1.血常规  2.肝功能  3.心电图  4.胸片  5.腹部彩超  6.CT  7.核磁  8.B超\n";
    std::string cat = inputLine("  检查类别（也可输入其他）：", 20);
    std::string items = inputLine("  检查项目描述：", MAX_TEXT_LEN);
    double fee = inputMoney("  检查费用（元）：");
    std::string err;
    const Doctor* doc = h.findDoctor(s.id);
    std::string id = h.createExamination(p->id, s.id, doc ? doc->deptId : "", cat, items, fee, err);


    if (id.empty()) std::cout << "  开单失败：" << err << "\n";
    else std::cout << "  检查单已开出：" << id << "，费用已计入患者账单。\n";
}

void doctorReportEntry(Hospital& h, const Session& s) {
    std::cout << "================ 检查单与报告 ================\n";
    std::vector<std::string> examIds;
    int i = 1;
    for (const auto& e : h.store.exams) {
        const Doctor* doc = h.findDoctor(e.doctorId);
        if (doc && doc->id != s.id) continue;
        const Patient* p = h.findPatient(e.patientId);
        std::cout << "  " << i++ << ". " << e.id << " " << (p ? p->name : "-") << " "
                  << e.category << " " << e.items << " " << e.status
                  << (h.findReportByExam(e.id) ? "（已有报告）" : "") << "\n";
        examIds.push_back(e.id);
    }
    if (examIds.empty()) { std::cout << "  （暂无检查单）\n"; return; }
    int c = inputInt("  选择要录入报告的检查单（0=返回）：", 0, (int)examIds.size());
    if (c == 0) return;
    if (h.findReportByExam(examIds[c - 1])) { std::cout << "  该检查已有报告。\n"; return; }
    std::string result = inputLine("  检查结果：", MAX_TEXT_LEN);
    std::string conclusion = inputLine("  报告结论：", MAX_TEXT_LEN);
    std::string err;
    if (h.addReport(examIds[c - 1], result, conclusion, s.id, err)) std::cout << "  报告已录入。\n";
    else std::cout << "  录入失败：" << err << "\n";
}

void doctorCreateRx(Hospital& h, const Session& s) {
    const Patient* p = pickPatient(h);
    if (!p) return;
    const Doctor* doc = h.findDoctor(s.id);
    std::vector<PrescriptionItem> items;
    while (true) {
        const Drug* d = pickDrug(h);
        if (!d) break;
        int qty = inputInt("  数量：", 1, 999);
        std::string usage = inputLine("  用法用量：", MAX_TEXT_LEN);
        PrescriptionItem it;
        it.drugId = d->id;
        it.qty = qty;
        it.usage = usage;
        items.push_back(it);
        if (inputYesNo("  继续添加药品？") != "y") break;
    }
    if (items.empty()) return;
    std::string err;
    std::string id = h.createPrescription(p->id, s.id, doc ? doc->deptId : "", items, err);
    if (id.empty()) std::cout << "  开方失败：" << err << "\n";
    else std::cout << "  处方已开具：" << id << "（待药剂师发药）\n";
}

void doctorDischarge(Hospital& h, const Session& s) {
    std::cout << "================ 我的在院患者 ================\n";
    std::vector<std::string> hosIds;
    int i = 1;
    for (const auto& hh : h.store.hospitalizations) {
        if (hh.doctorId != s.id || hh.status != "住院中") continue;
        const Patient* p = h.findPatient(hh.patientId);
        const Ward* w = h.findWard(hh.wardId);
        std::cout << "  " << i++ << ". " << hh.id << " " << (p ? p->name : "-") << " "
                  << (w ? w->name : "-") << " " << hh.bedId << " 入院:" << hh.admitDate << "\n";
        hosIds.push_back(hh.id);
    }
    if (hosIds.empty()) { std::cout << "  （暂无在院患者）\n"; return; }
    int c = inputInt("  选择办理出院的住院单（0=返回）：", 0, (int)hosIds.size());
    if (c == 0) return;
    if (inputYesNo("  确认办理出院并结算床位费？") != "y") return;
    std::string err;
    if (h.discharge(hosIds[c - 1], s.id, err)) std::cout << "  已办理出院，床位费已计入账单。\n";
    else std::cout << "  出院失败：" << err << "\n";
}

void doctorMenu(Hospital& h, const Session& s) {
    while (true) {
        clearScreen();
        banner();
        const Doctor* doc = h.findDoctor(s.id);
        std::cout << "---------- 医生工作站 ----------\n";
        if (doc) printPerson(*doc);
        std::cout << "  1. 我的患者（今日挂号/在院）\n";
        std::cout << "  2. 接诊看诊（书写病历）\n";
        std::cout << "  3. 开检查单\n";
        std::cout << "  4. 检查报告管理\n";
        std::cout << "  5. 开处方\n";
        std::cout << "  6. 我的处方列表\n";
        std::cout << "  7. 在院患者管理（办理出院）\n";
        std::cout << "  8. 查询患者信息\n";
        std::cout << "  9. 我的接诊统计\n";
        std::cout << "  10. 修改密码\n";
        std::cout << "  0. 退出登录\n";
        int c = inputInt("  请选择：", 0, 10);
        switch (c) {
            case 1: std::cout << h.reportDoctorPatients(s.id); break;
            case 2: doctorConsult(h, s); break;
            case 3: doctorCreateExam(h, s); break;
            case 4: doctorReportEntry(h, s); break;
            case 5: doctorCreateRx(h, s); break;
            case 6: {
                int i = 1;
                for (const auto& rx : h.store.prescriptions) {
                    if (rx.doctorId != s.id) continue;
                    const Patient* p = h.findPatient(rx.patientId);
                    std::cout << "  " << i++ << ". " << rx.id << " " << (p ? p->name : "-")
                              << " " << rx.date << " " << rx.status << " ￥" << fmtMoney(h.rxTotal(rx)) << "\n";
                }
                if (i == 1) std::cout << "  （暂无处方）\n";
                break;
            }
            case 7: doctorDischarge(h, s); break;
            case 8: {
                std::string kw = inputLine("  查询关键词（姓名/编号/身份证）：", MAX_IDCARD_LEN + 4);
                std::cout << h.reportByPatient(kw);
                break;
            }
            case 9: {
                int total = 0, done = 0;
                for (const auto& r : h.store.registrations)
                    if (r.doctorId == s.id) { ++total; if (r.status == RS_DONE) ++done; }
                std::cout << "  累计挂号：" << total << "，已就诊：" << done << "\n";
                break;
            }
            case 10: {
                std::string oldPw = inputLine("  原密码：", MAX_PASSWORD_LEN);
                std::string newPw = inputLine("  新密码（4~20位）：", MAX_PASSWORD_LEN);
                std::string err;
                if (h.changePassword(s.id, s.role, oldPw, newPw, err)) std::cout << "  密码修改成功。\n";
                else std::cout << "  修改失败：" << err << "\n";
                break;
            }
            case 0: return;
        }
        pause();
    }
}

} // namespace

// ============================================================
// 护士菜单（第4块）
// ============================================================
namespace {

void nurseAdmit(Hospital& h, const Session& s) {
    (void)s;
    const Patient* p = pickPatient(h);
    if (!p) return;
    const Department* dep = pickDept(h);
    if (!dep) return;
    const Doctor* doc = pickDoctor(h, dep->id);
    if (!doc) return;
    const Ward* ward = pickWard(h, true);
    if (!ward) return;
    std::string err;
    std::string id = h.admit(p->id, dep->id, doc->id, ward->id, err);
    if (id.empty()) std::cout << "  入院办理失败：" << err << "\n";
    else std::cout << "  入院成功！住院单号：" << id << "，已分配 " << ward->name << " 床位。\n";
}

void nurseDischarge(Hospital& h, const Session& s) {
    std::cout << "================ 在院患者（出院办理） ================\n";
    std::vector<std::string> hosIds;
    int i = 1;
    for (const auto& hh : h.store.hospitalizations) {
        if (hh.status != "住院中") continue;
        const Patient* p = h.findPatient(hh.patientId);
        const Ward* w = h.findWard(hh.wardId);
        std::cout << "  " << i++ << ". " << hh.id << " " << (p ? p->name : "-") << " "
                  << (w ? w->name : "-") << " " << hh.bedId << " 入院:" << hh.admitDate << "\n";
        hosIds.push_back(hh.id);
    }
    if (hosIds.empty()) { std::cout << "  （暂无在院患者）\n"; return; }
    int c = inputInt("  选择办理出院的住院单（0=返回）：", 0, (int)hosIds.size());
    if (c == 0) return;
    if (inputYesNo("  确认办理出院并结算床位费？") != "y") return;
    std::string err;
    if (h.discharge(hosIds[c - 1], s.id, err)) std::cout << "  已办理出院，床位进入清洁状态。\n";
    else std::cout << "  出院失败：" << err << "\n";
}

void nurseBedManage(Hospital& h, const Session& s) {
    (void)s;
    std::cout << h.reportBedOccupancy();
    std::string bedId = inputLine("  输入要修改状态的床位号（如 W001-01，0=返回）：", 20);
    if (bedId == "0") return;
    const Bed* bed = h.findBed(bedId);
    if (!bed) { std::cout << "  未找到该床位。\n"; return; }
    std::cout << "  当前状态：" << bed->status << "\n";
    std::cout << "  1. 空闲  2. 清洁中  3. 维修中\n";
    int c = inputInt("  请选择新状态：", 1, 3);
    std::string st = c == 1 ? BS_FREE : c == 2 ? BS_CLEANING : BS_REPAIR;
    std::string err;
    if (h.setBedStatus(bedId, st, err)) std::cout << "  床位状态已更新。\n";
    else std::cout << "  更新失败：" << err << "\n";
}

void nurseRecord(Hospital& h, const Session& s) {
    std::cout << "================ 在院患者（护理记录） ================\n";
    std::vector<std::string> pids;
    int i = 1;
    for (const auto& hh : h.store.hospitalizations) {
        if (hh.status != "住院中") continue;
        const Patient* p = h.findPatient(hh.patientId);
        std::cout << "  " << i++ << ". " << hh.patientId << " " << (p ? p->name : "-")
                  << " 床位:" << hh.bedId << "\n";
        pids.push_back(hh.patientId);
    }
    if (pids.empty()) { std::cout << "  （暂无在院患者）\n"; return; }
    int c = inputInt("  选择患者（0=返回）：", 0, (int)pids.size());
    if (c == 0) return;
    std::string content = inputLine("  护理记录内容：", MAX_TEXT_LEN);
    std::string err;
    std::string id = h.addNursingRecord(pids[c - 1], s.id, content, err);
    if (id.empty()) std::cout << "  记录失败：" << err << "\n";
    else std::cout << "  护理记录已保存：" << id << "\n";
}

void nurseMenu(Hospital& h, const Session& s) {
    while (true) {
        clearScreen();
        banner();
        const Nurse* n = h.findNurse(s.id);
        std::cout << "---------- 护士工作站 ----------\n";
        if (n) printPerson(*n);
        std::cout << "  1. 在院患者列表\n";
        std::cout << "  2. 办理入院（分配床位）\n";
        std::cout << "  3. 办理出院\n";
        std::cout << "  4. 床位总览\n";
        std::cout << "  5. 床位状态管理\n";
        std::cout << "  6. 护理记录录入\n";
        std::cout << "  7. 今日护理记录\n";
        std::cout << "  0. 退出登录\n";
        int c = inputInt("  请选择：", 0, 7);
        switch (c) {
            case 1: {
                for (const auto& hh : h.store.hospitalizations) {
                    if (hh.status != "住院中") continue;
                    const Patient* p = h.findPatient(hh.patientId);
                    const Ward* w = h.findWard(hh.wardId);
                    const Doctor* doc = h.findDoctor(hh.doctorId);
                    std::cout << "  " << hh.id << " " << (p ? p->name : "-") << " "
                              << (w ? w->name : "-") << " " << hh.bedId << " "
                              << (doc ? doc->name : "-") << " 入院:" << hh.admitDate << "\n";
                }
                break;
            }
            case 2: nurseAdmit(h, s); break;
            case 3: nurseDischarge(h, s); break;
            case 4: std::cout << h.reportBedOccupancy(); break;
            case 5: nurseBedManage(h, s); break;
            case 6: nurseRecord(h, s); break;
            case 7: {
                int i = 1;
                for (const auto& nr : h.store.nursingRecords) {
                    if (nr.date != today()) continue;
                    const Patient* p = h.findPatient(nr.patientId);
                    const Nurse* nn = h.findNurse(nr.nurseId);
                    std::cout << "  " << i++ << ". " << nr.time << " " << (p ? p->name : "-")
                              << " 床位:" << nr.bedId << " 护士:" << (nn ? nn->name : "-")
                              << " " << nr.content << "\n";
                }
                if (i == 1) std::cout << "  （今日暂无护理记录）\n";
                break;
            }
            case 0: return;
        }
        pause();
    }
}

} // namespace

// ============================================================
// 药剂师菜单（第5块）
// ============================================================
namespace {

void phDispense(Hospital& h, const Session& s) {
    std::cout << h.reportPendingRx();
    std::string rxId = inputLine("  输入要发药的处方号（0=返回）：", 20);
    if (rxId == "0") return;
    if (!h.findRx(rxId)) { std::cout << "  未找到该处方。\n"; return; }
    if (inputYesNo("  确认审核通过并发药？") != "y") return;
    std::string err;
    if (h.dispensePrescription(rxId, s.id, err)) {
        std::cout << "  发药成功，库存已扣减，出入库台账已记录。\n";
    } else {
        std::cout << "  发药失败：" << err << "\n";
    }
}

void phStockIn(Hospital& h, const Session& s) {
    const Drug* d = pickDrug(h);
    if (!d) return;
    int qty = inputInt("  入库数量：", 1, 99999);
    std::string remark = inputLine("  备注（可空，回车跳过）：", MAX_TEXT_LEN, true);
    std::string err;
    if (h.stockIn(d->id, qty, s.id, remark, err)) std::cout << "  入库成功，当前库存：" << d->stock + qty << "\n";
    else std::cout << "  入库失败：" << err << "\n";
}

void phStockOut(Hospital& h, const Session& s) {
    const Drug* d = pickDrug(h);
    if (!d) return;
    std::cout << "  出库类型：1. 退货  2. 损耗\n";
    int t = inputInt("  请选择：", 1, 2);
    std::string type = t == 1 ? ST_RETURN : ST_LOSS;
    int qty = inputInt("  出库数量：", 1, d->stock);
    std::string remark = inputLine("  备注（可空，回车跳过）：", MAX_TEXT_LEN, true);
    std::string err;
    if (h.stockOut(d->id, qty, type, s.id, remark, err)) std::cout << "  出库成功，当前库存：" << d->stock - qty << "\n";
    else std::cout << "  出库失败：" << err << "\n";
}

void phStockList(Hospital& h) {
    std::cout << "================ 药品库存一览 ================\n";
    std::vector<int> ws = {9, 18, 16, 12, 10, 8, 8};
    std::cout << hm::padRight("编号", ws[0]) << "  " << hm::padRight("通用名", ws[1]) << "  "
              << hm::padRight("商品名", ws[2]) << "  " << hm::padRight("分类", ws[3]) << "  "
              << hm::padRight("库存", ws[4]) << "  " << hm::padRight("下限", ws[5]) << "  "
              << hm::padRight("单价", ws[6]) << "\n";
    for (const auto& d : h.store.drugs) {
        std::string flag = d.stock <= d.minStock ? " [预警]" : "";
        std::cout << hm::padRight(d.id, ws[0]) << "  " << hm::padRight(d.genericName, ws[1]) << "  "
                  << hm::padRight(d.brandName, ws[2]) << "  " << hm::padRight(d.category, ws[3]) << "  "
                  << hm::padRight(std::to_string(d.stock), ws[4]) << "  "
                  << hm::padRight(std::to_string(d.minStock), ws[5]) << "  "
                  << hm::padRight(fmtMoney(d.price), ws[6]) << flag << "\n";
    }
}

void phStockLog(Hospital& h) {
    std::cout << "================ 出入库台账（最近 60 条） ================\n";
    int shown = 0;
    size_t start = h.store.stockRecords.size() > 60 ? h.store.stockRecords.size() - 60 : 0;
    for (size_t i = start; i < h.store.stockRecords.size(); ++i) {
        const auto& sr = h.store.stockRecords[i];
        const Drug* d = h.findDrug(sr.drugId);
        std::cout << "  " << sr.date << " " << sr.id << " " << (d ? d->genericName : sr.drugId)
                  << " " << sr.type << " " << sr.qty << " 操作人:" << sr.operatorName
                  << (sr.remark.empty() ? "" : " 备注:" + sr.remark) << "\n";
        ++shown;
    }
    if (shown == 0) std::cout << "  （暂无记录）\n";
}

void pharmacistMenu(Hospital& h, const Session& s) {
    while (true) {
        clearScreen();
        banner();
        const Pharmacist* ph = nullptr;
        for (const auto& x : h.store.pharmacists) if (x.id == s.id) ph = &x;
        std::cout << "---------- 药房工作站 ----------\n";
        if (ph) printPerson(*ph);
        std::cout << "  1. 处方审核与发药\n";
        std::cout << "  2. 处方查询\n";
        std::cout << "  3. 药品库存一览\n";
        std::cout << "  4. 库存预警\n";
        std::cout << "  5. 药品入库\n";
        std::cout << "  6. 药品出库（退货/损耗）\n";
        std::cout << "  7. 出入库台账\n";
        std::cout << "  8. 药品字典查询（通用名/商品名/别名）\n";
        std::cout << "  0. 退出登录\n";
        int c = inputInt("  请选择：", 0, 8);
        switch (c) {
            case 1: phDispense(h, s); break;
            case 2: {
                std::string kw = inputLine("  输入处方号（0=返回）：", 20);
                if (kw == "0") break;
                for (const auto& rx : h.store.prescriptions) {
                    if (rx.id != kw) continue;
                    const Patient* p = h.findPatient(rx.patientId);
                    const Doctor* doc = h.findDoctor(rx.doctorId);
                    std::cout << rx.id << " 患者:" << (p ? p->name : "-") << " 医生:"
                              << (doc ? doc->name : "-") << " 日期:" << rx.date
                              << " 状态:" << rx.status << " 金额:￥" << fmtMoney(h.rxTotal(rx)) << "\n";
                    for (const auto& it : rx.items) {
                        const Drug* g = h.findDrug(it.drugId);
                        std::cout << "    - " << (g ? g->genericName : it.drugId)
                                  << " x" << it.qty << " " << it.usage << "\n";
                    }
                }
                break;
            }
            case 3: phStockList(h); break;
            case 4: std::cout << h.reportLowStock(); break;
            case 5: phStockIn(h, s); break;
            case 6: phStockOut(h, s); break;
            case 7: phStockLog(h); break;
            case 8: {
                std::string kw = inputLine("  查询关键词：", MAX_DRUG_NAME_LEN + 4);
                std::cout << h.reportByDrug(kw);
                break;
            }
            case 0: return;
        }
        pause();
    }
}

} // namespace

// ============================================================
// 管理员菜单（第6块）
// ============================================================
namespace {

void adminReportCenter(Hospital& h) {
    while (true) {
        clearScreen();
        banner();
        std::cout << "---------- 报表中心 ----------\n";
        std::cout << "  1. 患者视角（按患者查询记录/账单）\n";
        std::cout << "  2. 医护视角（门诊/接诊量/床位/预警/待发药）\n";
        std::cout << "  3. 管理视角（收入/药品/周转/流量）\n";
        std::cout << "  4. 基础信息查询（患者/医生/科室/药品）\n";
        std::cout << "  0. 返回\n";
        int c = inputInt("  请选择：", 0, 4);
        if (c == 0) return;
        if (c == 1) {
            std::string kw = inputLine("  输入患者姓名/编号/身份证号：", MAX_IDCARD_LEN + 4);
            auto list = h.searchPatients(kw);
            if (list.empty()) { std::cout << "  未找到患者。\n"; }
            else {
                const Patient* p = list.size() == 1 ? list[0] : nullptr;
                if (!p) {
                    for (size_t i = 0; i < list.size(); ++i)
                        std::cout << "  " << (i + 1) << ". " << list[i]->id << " " << list[i]->name
                                  << " " << list[i]->gender << " " << list[i]->age << "岁 身份证尾号:"
                                  << (list[i]->idCard.size() >= 4 ? list[i]->idCard.substr(list[i]->idCard.size() - 4) : "-") << "\n";
                    int sel = inputInt("  选择患者（0=取消）：", 0, (int)list.size());
                    if (sel == 0) { pause(); continue; }
                    p = list[sel - 1];
                }
                std::cout << h.reportPatientInfo(p->id);
                std::cout << h.reportPatientRecords(p->id);
                std::cout << h.reportPatientExams(p->id);
                std::cout << h.reportPatientRx(p->id);
                std::cout << h.reportPatientHos(p->id);
                std::cout << h.reportPatientBills(p->id);
            }
        } else if (c == 2) {
            std::cout << h.reportDailyOutpatient();
            std::cout << h.reportDoctorWorkload();
            std::cout << h.reportBedOccupancy();
            std::cout << h.reportLowStock();
            std::cout << h.reportPendingRx();
        } else if (c == 3) {
            std::cout << h.reportDeptRevenue();
            std::cout << h.reportDrugFlow();
            std::cout << h.reportDrugSales();
            std::cout << h.reportBedTurnover();
            std::cout << h.reportPatientFlow();
        } else {
            std::cout << "  1. 按患者  2. 按医生  3. 按科室  4. 按药品\n";
            int q = inputInt("  请选择查询方式：", 1, 4);
            std::string kw = inputLine("  查询关键词：", 40);
            if (q == 1) std::cout << h.reportByPatient(kw);
            else if (q == 2) std::cout << h.reportByDoctor(kw);
            else if (q == 3) std::cout << h.reportByDept(kw);
            else std::cout << h.reportByDrug(kw);
        }
        pause();
    }
}

void adminDept(Hospital& h) {
    int i = 1;
    for (const auto& d : h.store.departments)
        std::cout << "  " << i++ << ". " << d.id << " " << d.name << " " << d.location
                  << " 关联病房:" << d.wardType << "\n";
    if (inputYesNo("  是否新增科室？") != "y") return;
    Department d;
    d.name = inputLine("  科室名称（<=20字符）：", MAX_DEPT_NAME_LEN);
    d.location = inputLine("  位置：", MAX_DEPT_NAME_LEN);
    d.desc = inputLine("  简介（可空）：", MAX_TEXT_LEN, true);
    std::cout << "  关联病房类型：1.普通 2.ICU 3.隔离 4.无\n";
    int t = inputInt("  请选择：", 1, 4);
    d.wardType = t == 1 ? WT_GENERAL : t == 2 ? WT_ICU : t == 3 ? WT_ISOLATION : "无";
    std::string err;
    if (h.addDepartment(d, err)) std::cout << "  新增成功：" << d.id << " " << d.name << "\n";
    else std::cout << "  新增失败：" << err << "\n";
}

void adminDoctor(Hospital& h) {
    int i = 1;
    for (const auto& d : h.store.doctors) {
        const Department* dep = h.findDept(d.deptId);
        std::cout << "  " << i++ << ". " << d.id << " " << d.name << " " << (dep ? dep->name : "-")
                  << " " << d.title << " 接诊量：";
        int done = 0;
        for (const auto& r : h.store.registrations) if (r.doctorId == d.id && r.status == RS_DONE) ++done;
        std::cout << done << "\n";
    }
    if (inputYesNo("  是否新增医生？") != "y") return;
    Doctor d;
    d.name = inputLine("  姓名：", MAX_NAME_LEN);
    std::cout << "  性别：1.男 2.女\n";
    d.gender = inputInt("  请选择：", 1, 2) == 1 ? "男" : "女";
    d.age = inputInt("  年龄：", 20, 70);
    d.idCard = inputIdCard("  身份证号：");
    d.phone = inputPhone("  电话（11位手机号）：");
    d.username = inputLine("  登录账号（默认同编号，可自定义）：", MAX_USERNAME_LEN);
    printDepts(h);
    int c = inputInt("  选择所属科室：", 1, (int)h.store.departments.size());
    d.deptId = h.store.departments[c - 1].id;
    std::cout << "  职称：1.主任医师 2.副主任医师 3.主治医师 4.医师\n";
    int t = inputInt("  请选择：", 1, 4);
    static const char* TITLES[4] = {"主任医师", "副主任医师", "主治医师", "医师"};
    d.title = TITLES[t - 1];
    d.specialty = inputLine("  专长：", MAX_TEXT_LEN);
    std::string err;
    if (h.addDoctor(d, err)) std::cout << "  新增成功：" << d.id << " " << d.name << "（密码 123456）\n";
    else std::cout << "  新增失败：" << err << "\n";
}

void adminDrug(Hospital& h) {
    std::cout << "================ 药品字典 ================\n";
    std::cout << h.reportByDrug("");
    if (inputYesNo("  是否新增药品？") != "y") return;
    Drug d;
    d.genericName = inputLine("  通用名：", MAX_DRUG_NAME_LEN);
    d.brandName = inputLine("  商品名（可空）：", MAX_DRUG_NAME_LEN, true);
    d.alias = inputLine("  别名（多个用/分隔，可空）：", MAX_ALIAS_LEN, true);
    d.category = inputLine("  分类：", MAX_DRUG_NAME_LEN);
    d.spec = inputLine("  规格：", MAX_DRUG_NAME_LEN);
    d.unit = inputLine("  单位：", 10);
    d.price = inputMoney("  单价（元）：");
    d.stock = inputInt("  初始库存：", 0, 999999);
    d.minStock = inputInt("  库存下限：", 0, 999999);
    d.manufacturer = inputLine("  生产厂家（可空）：", MAX_DRUG_NAME_LEN, true);
    d.expiry = inputLine("  有效期至（如2028-12，可空）：", 10, true);
    if (inputYesNo("  是否关联科室？") == "y") {
        const Department* dep = pickDept(h);
        if (dep) d.deptId = dep->id;
    }
    std::string err;
    if (h.addDrug(d, err)) std::cout << "  新增成功：" << d.id << " " << d.genericName << "\n";
    else std::cout << "  新增失败：" << err << "\n";
}

void adminWard(Hospital& h) {
    for (const auto& w : h.store.wards) {
        auto beds = h.bedsOfWard(w.id);
        std::cout << "  " << w.id << " " << w.name << "（" << w.type << "） ￥" << fmtMoney(w.bedFee)
                  << "/天 床位:" << beds.size();
        for (const auto* b : beds) std::cout << " " << b->id << ":" << b->status;
        std::cout << "\n";
    }
    std::cout << "  1. 新增病房  2. 新增床位  0. 返回\n";
    int c = inputInt("  请选择：", 0, 2);
    if (c == 0) return;
    if (c == 1) {
        Ward w;
        w.name = inputLine("  病房名称：", MAX_DEPT_NAME_LEN);
        std::cout << "  类型：1.普通 2.ICU 3.隔离\n";
        int t = inputInt("  请选择：", 1, 3);
        w.type = t == 1 ? WT_GENERAL : t == 2 ? WT_ICU : WT_ISOLATION;
        if (inputYesNo("  是否关联科室？") == "y") {
            const Department* dep = pickDept(h);
            if (dep) w.deptId = dep->id;
        }
        w.bedFee = inputMoney("  床位费（元/天）：");
        std::string err;
        if (h.addWard(w, err)) std::cout << "  新增成功：" << w.id << "，请继续新增床位。\n";
        else std::cout << "  新增失败：" << err << "\n";
    } else {
        const Ward* w = pickWard(h, false);
        if (!w) return;
        std::string err;
        if (h.addBed(w->id, err)) std::cout << "  新增床位成功。\n";
        else std::cout << "  新增失败：" << err << "\n";
    }
}

void adminResetPwd(Hospital& h) {
    std::cout << "  角色：1.患者 2.医生 3.护士 4.药剂师 5.管理员\n";
    int r = inputInt("  请选择：", 1, 5);
    std::string role = r == 1 ? ROLE_PATIENT : r == 2 ? ROLE_DOCTOR : r == 3 ? ROLE_NURSE
                    : r == 4 ? ROLE_PHARMACIST : ROLE_ADMIN;
    std::string kw = inputLine("  输入编号/姓名关键词：", MAX_NAME_LEN + 4);
    int shown = 0;
    std::vector<std::pair<std::string, std::string>> users;
    auto addUser = [&](const Person& p, const std::string& ro) {
        if (containsIgnoreCase(p.id, kw) || containsIgnoreCase(p.name, kw)) {
            std::cout << "  " << (shown + 1) << ". " << p.id << " " << p.name << "（" << ro << "）\n";
            users.push_back({p.id, ro});
            ++shown;
        }
    };
    for (const auto& p : h.store.patients) if (role == ROLE_PATIENT) addUser(p, ROLE_PATIENT);
    for (const auto& p : h.store.doctors) if (role == ROLE_DOCTOR) addUser(p, ROLE_DOCTOR);
    for (const auto& p : h.store.nurses) if (role == ROLE_NURSE) addUser(p, ROLE_NURSE);
    for (const auto& p : h.store.pharmacists) if (role == ROLE_PHARMACIST) addUser(p, ROLE_PHARMACIST);
    for (const auto& p : h.store.admins) if (role == ROLE_ADMIN) addUser(p, ROLE_ADMIN);
    if (users.empty()) { std::cout << "  未找到用户。\n"; return; }
    int c = inputInt("  选择要重置密码的用户（0=取消）：", 0, (int)users.size());
    if (c == 0) return;
    if (inputYesNo("  确认将密码重置为 123456？") != "y") return;
    std::string err;
    if (h.resetPassword(users[c - 1].first, users[c - 1].second, err))
        std::cout << "  已重置为 123456。\n";
    else std::cout << "  重置失败：" << err << "\n";
}

void adminMenu(Hospital& h, const Session& s) {
    while (true) {
        clearScreen();
        banner();
        const Admin* a = nullptr;
        for (const auto& x : h.store.admins) if (x.id == s.id) a = &x;
        std::cout << "---------- 管理员控制台 ----------\n";
        if (a) printPerson(*a);
        std::cout << "  1. 科室维护\n";
        std::cout << "  2. 医生维护\n";
        std::cout << "  3. 药品字典维护\n";
        std::cout << "  4. 病房床位维护\n";
        std::cout << "  5. 报表中心\n";
        std::cout << "  6. 数据规模校验\n";
        std::cout << "  7. 用户密码重置\n";
        std::cout << "  8. 全院运营总览\n";
        std::cout << "  0. 退出登录\n";
        int c = inputInt("  请选择：", 0, 8);
        switch (c) {
            case 1: adminDept(h); break;
            case 2: adminDoctor(h); break;
            case 3: adminDrug(h); break;
            case 4: adminWard(h); break;
            case 5: adminReportCenter(h); break;
            case 6: std::cout << h.scaleReport(); break;
            case 7: adminResetPwd(h); break;
            case 8: std::cout << h.reportAll(); break;
            case 0: return;
        }
        pause();
    }
}

} // namespace
} // namespace hm
