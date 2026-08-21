// ============================================================
// 医疗管理系统 - 演示数据生成（第1块：科室/人员/药品/病房）
// ============================================================
#include "seed.h"
#include <ctime>
#include <sstream>

namespace hm {

namespace {

// 确定性伪随机（保证每次生成的演示数据一致）
int rng(int& seed) { seed = seed * 1103515245 + 12345; return (seed >> 16) & 0x7fffffff; }
int rnd(int& seed, int lo, int hi) { return lo + rng(seed) % (hi - lo + 1); }

std::string dateOffset(int days) {
    std::time_t t = std::time(nullptr) - days * 86400;
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string padId(int n, int w) {
    std::string s = std::to_string(n);
    while ((int)s.size() < w) s = "0" + s;
    return s;
}

const char* SURNAMES[] = {"王", "李", "张", "刘", "陈", "杨", "赵", "黄", "周", "吴",
                          "徐", "孙", "马", "朱", "胡", "郭", "何", "高", "林", "罗"};
const char* GIVENS[] = {"伟", "芳", "娜", "敏", "静", "丽", "强", "磊", "军", "洋",
                        "勇", "艳", "杰", "涛", "明", "超", "霞", "平", "刚", "秀英",
                        "建华", "文", "辉", "玉兰", "红", "玉梅", "海燕", "晨", "宇", "欣怡",
                        "子涵", "浩然", "思远", "嘉怡", "梓豪", "若曦", "俊杰", "诗涵", "志强", "淑珍"};

// 身份证号生成（保证唯一）
std::string makeIdCard(int& seed, int seq) {
    int y = rnd(seed, 1960, 2008);
    int m = rnd(seed, 1, 12);
    int d = rnd(seed, 1, 28);
    std::ostringstream oss;
    oss << "110101" << y << padId(m, 2) << padId(d, 2) << padId(seq, 4);
    return oss.str();
}

std::string makePhone(int& seed) {
    std::ostringstream oss;
    oss << "1";
    for (int i = 0; i < 10; ++i) oss << rnd(seed, 0, 9);
    return oss.str();
}

// 人名生成（第 74~145 名患者刻意复用前 72 名患者的姓名，制造重名场景）
std::string makeName(int& seed, int idx) {
    (void)seed;
    if (idx >= 73 && idx <= 145) idx = idx - 72; // mirror 2..73 -> duplicate names
    int s = (idx * 7) % 20;
    int g = (idx * 13 + 5) % 40;
    return std::string(SURNAMES[s]) + GIVENS[g];
}

// 医生姓名（不重名）
std::string makeDoctorName(int& seed, int idx) {
    (void)idx;
    std::string n;
    do {
        n = std::string(SURNAMES[rng(seed) % 20]) + GIVENS[rng(seed) % 40];
    } while (false);
    return n;
}

const char* DEPT_SPECIALTY[5][2] = {
    {"内科", "心血管/呼吸/消化内科"},
    {"外科", "普外/肝胆/胃肠外科"},
    {"儿科", "小儿内科/儿保"},
    {"妇产科", "产科/妇科/围产保健"},
    {"骨科", "创伤/脊柱/关节骨科"},
};

const char* DEPT_LOCATION[5] = {"门诊楼2层", "门诊楼3层", "门诊楼1层", "门诊楼2层", "门诊楼3层"};
const char* DOCTOR_TITLES[4] = {"主任医师", "副主任医师", "主治医师", "医师"};

} // namespace

bool seedIfEmpty(DataStore& store, std::vector<std::string>& msgs) {
    int seed = 20260821;

    // ---------- 科室（5 个） ----------
    std::vector<std::string> deptIds;
    for (int i = 0; i < 5; ++i) {
        Department d;
        d.id = "DEPT0" + std::to_string(i + 1);
        d.name = DEPT_SPECIALTY[i][0];
        d.location = DEPT_LOCATION[i];
        d.desc = DEPT_SPECIALTY[i][1];
        d.wardType = (i < 5) ? WT_GENERAL : "无";
        store.departments.push_back(d);
        deptIds.push_back(d.id);
    }

    // ---------- 医生（每科室 4 名，共 20 名） ----------
    int doctorSeq = 0;
    for (int d = 0; d < 5; ++d) {
        for (int k = 0; k < 4; ++k) {
            Doctor doc;
            ++doctorSeq;
            doc.id = "D" + padId(doctorSeq, 5);
            doc.name = makeDoctorName(seed, doctorSeq);
            doc.gender = (k % 2 == 0) ? "男" : "女";
            doc.age = rnd(seed, 32, 58);
            doc.idCard = makeIdCard(seed, doctorSeq);
            doc.phone = makePhone(seed);
            doc.username = doc.id;
            doc.password = "123456";
            doc.role = ROLE_DOCTOR;
            doc.deptId = deptIds[d];
            doc.title = DOCTOR_TITLES[k];
            doc.specialty = DEPT_SPECIALTY[d][1];
            store.doctors.push_back(doc);
        }
    }

    // ---------- 护士 / 药剂师 / 管理员 ----------
    for (int i = 0; i < 10; ++i) {
        Nurse n;
        n.id = "N" + padId(i + 1, 5);
        n.name = makeDoctorName(seed, 100 + i);
        n.gender = "女";
        n.age = rnd(seed, 24, 45);
        n.idCard = makeIdCard(seed, 500 + i);
        n.phone = makePhone(seed);
        n.username = n.id;
        n.password = "123456";
        n.role = ROLE_NURSE;
        n.deptId = deptIds[i % 5];
        store.nurses.push_back(n);
    }
    for (int i = 0; i < 3; ++i) {
        Pharmacist ph;
        ph.id = "PH" + padId(i + 1, 5);
        ph.name = makeDoctorName(seed, 200 + i);
        ph.gender = (i % 2 == 0) ? "男" : "女";
        ph.age = rnd(seed, 26, 50);
        ph.idCard = makeIdCard(seed, 600 + i);
        ph.phone = makePhone(seed);
        ph.username = ph.id;
        ph.password = "123456";
        ph.role = ROLE_PHARMACIST;
        store.pharmacists.push_back(ph);
    }
    Admin adm;
    adm.id = "A00001";
    adm.name = "系统管理员";
    adm.gender = "男";
    adm.age = 35;
    adm.idCard = "110101198001010011";
    adm.phone = "13800000000";
    adm.username = "admin";
    adm.password = "123456";
    adm.role = ROLE_ADMIN;
    store.admins.push_back(adm);

    // ---------- 药品（26 种，覆盖 12 类以上，部分关联科室） ----------
    // 字段：通用名, 商品名, 别名, 分类, 规格, 单位, 关联科室(可为空), 生产厂家, 单价, 初始库存, 库存下限
    struct DrugSeed {
        const char* gen; const char* brand; const char* alias; const char* cat;
        const char* spec; const char* unit; const char* dept;
        const char* mfr; double price; int stock; int minStock;
    };
    const DrugSeed DS[] = {
        {"阿莫西林胶囊", "阿莫仙", "羟氨苄青霉素", "抗生素类", "0.25g*24粒", "盒", "DEPT01", "华北制药", 8.50, 320, 60},
        {"头孢克肟分散片", "世福素", "头孢克肟", "抗生素类", "50mg*8片", "盒", "DEPT01", "广州白云山", 15.00, 180, 40},
        {"布洛芬缓释胶囊", "芬必得", "异丁苯丙酸", "解热镇痛类", "0.3g*20粒", "盒", "", "中美史克", 12.50, 260, 50},
        {"对乙酰氨基酚片", "泰诺林", "扑热息痛", "解热镇痛类", "0.5g*12片", "盒", "DEPT03", "强生制药", 6.80, 140, 30},
        {"氨溴索口服液", "沐舒坦", "盐酸氨溴索", "呼吸系统用药", "100ml", "瓶", "DEPT03", "勃林格殷格翰", 18.00, 90, 25},
        {"复方甘草片", "无", "甘草片", "呼吸系统用药", "100片", "瓶", "DEPT01", "同仁堂", 9.00, 120, 30},
        {"硝苯地平缓释片", "拜新同", "心痛定", "降压药", "30mg*7片", "盒", "DEPT01", "拜耳医药", 28.00, 110, 30},
        {"苯磺酸氨氯地平片", "络活喜", "氨氯地平", "降压药", "5mg*7片", "盒", "DEPT01", "辉瑞制药", 32.00, 95, 25},
        {"二甲双胍片", "格华止", "甲福明", "降糖药", "0.5g*20片", "盒", "DEPT01", "中美上海施贵宝", 22.00, 130, 30},
        {"阿卡波糖片", "拜唐苹", "卡博平", "降糖药", "50mg*30片", "盒", "DEPT01", "拜耳医药", 41.00, 85, 20},
        {"奥美拉唑肠溶胶囊", "洛赛克", "奥克", "消化系统用药", "20mg*14粒", "盒", "DEPT01", "阿斯利康", 26.00, 100, 25},
        {"蒙脱石散", "思密达", "双八面体蒙脱石", "消化系统用药", "3g*10袋", "盒", "DEPT03", "博福-益普生", 19.50, 80, 20},
        {"阿司匹林肠溶片", "拜阿司匹灵", "乙酰水杨酸", "心血管用药", "100mg*30片", "盒", "DEPT01", "拜耳医药", 14.00, 200, 40},
        {"阿托伐他汀钙片", "立普妥", "阿托伐他汀", "心血管用药", "20mg*7片", "盒", "DEPT01", "辉瑞制药", 45.00, 70, 20},
        {"云南白药气雾剂", "云南白药", "白药喷雾", "骨科/外用", "85g", "瓶", "DEPT05", "云南白药集团", 29.80, 150, 35},
        {"骨康胶囊", "骨康", "无", "骨科用药", "0.4g*36粒", "盒", "DEPT05", "贵州百灵", 23.00, 110, 25},
        {"布洛芬乳膏", "扶他林", "双氯芬酸二乙胺乳胶", "外用药", "20g", "支", "DEPT05", "诺华制药", 24.50, 90, 20},
        {"双氯芬酸钠缓释片", "扶他林", "双氯芬酸", "解热镇痛类", "75mg*10片", "盒", "DEPT05", "诺华制药", 16.00, 105, 25},
        {"红霉素软膏", "无", "红霉素", "外用药", "10g", "支", "", "上海通用药业", 3.50, 200, 40},
        {"维生素C片", "力度伸", "抗坏血酸", "维生素类", "1g*10片", "盒", "DEPT03", "拜耳医药", 36.00, 160, 35},
        {"复合维生素B片", "无", "维B", "维生素类", "100片", "瓶", "", "华中药业", 4.80, 240, 50},
        {"氯雷他定片", "开瑞坦", "氯羟他定", "抗过敏药", "10mg*6片", "盒", "", "拜耳医药", 17.00, 130, 30},
        {"叶酸片", "斯利安", "维生素M", "妇产科用药", "0.4mg*31片", "盒", "DEPT04", "北京斯利安", 13.00, 120, 30},
        {"缩宫素注射液", "催产素", "催产素", "妇产科用药", "10单位", "支", "DEPT04", "上海第一生化", 8.20, 60, 15},
        {"小儿止咳糖浆", "无", "止咳糖浆", "呼吸系统用药", "100ml", "瓶", "DEPT03", "葵花药业", 11.00, 85, 20},
        {"葡萄糖酸钙口服液", "三精葡萄糖酸钙", "补钙口服液", "营养补充类", "10ml*10支", "盒", "DEPT05", "哈药三精", 15.80, 140, 30},
    };
    for (const auto& ds : DS) {
        Drug d;
        d.id = store.nextId("DRUG");
        d.genericName = ds.gen;
        d.brandName = ds.brand;
        d.alias = ds.alias;
        d.category = ds.cat;
        d.spec = ds.spec;
        d.unit = ds.unit;
        d.deptId = ds.dept;
        d.manufacturer = ds.mfr;
        d.expiry = "2028-12";
        d.price = ds.price;
        d.stock = ds.stock;
        d.minStock = ds.minStock;
        store.drugs.push_back(d);
        if (!d.deptId.empty()) {
            for (auto& dep : store.departments)
                if (dep.id == d.deptId) dep.drugIds.push_back(d.id);
        }
    }

    // ---------- 病房（7 间：5 间关联科室 + ICU + 隔离）与床位（52 张） ----------
    struct WardSeed { const char* id; const char* name; const char* type; const char* dept; double fee; int beds; };
    const WardSeed WS[] = {
        {"W001", "内科普通病房", WT_GENERAL.c_str(), "DEPT01", 80, 10},
        {"W002", "外科普通病房", WT_GENERAL.c_str(), "DEPT02", 90, 10},
        {"W003", "儿科普通病房", WT_GENERAL.c_str(), "DEPT03", 70, 6},
        {"W004", "妇产科病房", WT_GENERAL.c_str(), "DEPT04", 75, 6},
        {"W005", "骨科普通病房", WT_GENERAL.c_str(), "DEPT05", 85, 8},
        {"W006", "重症监护病房(ICU)", WT_ICU.c_str(), "", 1500, 6},
        {"W007", "隔离病房", WT_ISOLATION.c_str(), "", 200, 6},
    };
    for (const auto& ws : WS) {
        Ward w;
        w.id = ws.id;
        w.name = ws.name;
        w.type = ws.type;
        w.deptId = ws.dept;
        w.desc = ws.name;
        w.bedFee = ws.fee;
        store.wards.push_back(w);
        for (int k = 1; k <= ws.beds; ++k) {
            Bed b;
            b.id = w.id + "-" + padId(k, 2);
            b.wardId = w.id;
            b.status = BS_FREE;
            store.beds.push_back(b);
        }
    }

    // ---------- 患者（145 名：113 门诊 + 32 住院；含 72 对重名） ----------
    int totalPatients = 145;
    std::vector<std::string> patientIds;
    for (int i = 1; i <= totalPatients; ++i) {
        Patient p;
        p.id = "P" + padId(i, 5);
        p.name = makeName(seed, i);
        p.gender = (i % 2 == 0) ? "女" : "男";
        if (i % 9 == 0) p.age = rnd(seed, 3, 12);        // 儿童（儿科场景）
        else if (i % 7 == 0) p.age = rnd(seed, 20, 35);  // 青壮年（妇产科场景）
        else p.age = rnd(seed, 28, 78);
        p.idCard = makeIdCard(seed, i);
        p.phone = makePhone(seed);
        p.username = p.id;
        p.password = "123456";
        p.role = ROLE_PATIENT;
        p.bloodType = (i % 4 == 0) ? "A" : (i % 4 == 1) ? "B" : (i % 4 == 2) ? "O" : "AB";
        p.allergy = (i % 11 == 0) ? "青霉素过敏" : (i % 13 == 0) ? "磺胺类过敏" : "无";
        p.address = "示例市幸福路" + std::to_string(100 + i) + "号";
        p.regDate = dateOffset(rnd(seed, 30, 400));
        p.status = (i <= 32) ? PS_INPATIENT : PS_OUTPATIENT;
        store.patients.push_back(p);
        patientIds.push_back(p.id);
    }

    // ---------- 出入库台账：药品初始入库 ----------
    for (const auto& d : store.drugs) {
        StockRecord sr;
        sr.id = store.nextId("ST");
        sr.drugId = d.id;
        sr.type = ST_IN;
        sr.date = dateOffset(rnd(seed, 10, 90));
        sr.operatorName = "药剂师PH00001";
        sr.remark = "期初入库";
        sr.qty = d.stock;
        store.stockRecords.push_back(sr);
    }

    // ---------- 挂号与看诊（第4块） ----------
    auto mkBill = [&](const std::string& pid, const std::string& date, bool paid, const std::vector<BillItem>& items) {
        Bill b;
        b.id = store.nextId("BILL");
        b.patientId = pid;
        b.date = date;
        b.status = paid ? BI_PAID : BI_UNPAID;
        b.payDate = paid ? date : "";
        b.items = items;
        store.bills.push_back(b);
    };

    const char* CHIEFS[] = {"发热咳嗽3天", "头痛头晕1周", "腹痛腹泻2天", "腰腿疼痛反复", "胸闷气短", "皮肤瘙痒",
                            "外伤疼痛", "体检发现血压偏高", "胃部不适反酸", "关节疼痛", "乏力纳差", "心悸心慌"};
    const char* DIAGS[] = {"上呼吸道感染", "高血压", "急性肠胃炎", "腰椎间盘突出", "冠心病待查", "过敏性皮炎",
                           "软组织损伤", "原发性高血压", "慢性胃炎", "骨关节炎", "贫血待查", "心律失常待查"};
    const char* ADVICES[] = {"多饮水，注意休息，按时服药，不适随诊", "低盐低脂饮食，监测血压，定期复诊",
                             "清淡饮食，注意腹部保暖", "减少负重，避免久坐，建议理疗", "完善检查后复诊",
                             "避免接触过敏原，注意皮肤保湿", "局部制动，注意观察", "规律服药，控制饮食",
                             "规律三餐，忌辛辣刺激", "适度运动，注意保暖", "加强营养，复查血常规",
                             "保持情绪稳定，避免劳累"};
    const char* EXAM_CATS[5][4] = {
        {"血常规", "肝功能", "心电图", "胃镜"},
        {"胸片", "腹部彩超", "CT", "血常规"},
        {"血常规", "微量元素", "胸片", "过敏原检测"},
        {"B超", "产前检查", "白带常规", "性激素六项"},
        {"X光", "CT", "核磁", "骨密度"},
    };
    auto examFee = [](const std::string& cat) -> double {
        static const std::map<std::string, double> feeMap = {
            {"血常规", 30}, {"肝功能", 80}, {"心电图", 40}, {"胃镜", 220},
            {"胸片", 60}, {"腹部彩超", 120}, {"CT", 280}, {"核磁", 480},
            {"微量元素", 50}, {"过敏原检测", 100}, {"B超", 100}, {"产前检查", 120},
            {"白带常规", 40}, {"性激素六项", 160}, {"X光", 60}, {"骨密度", 80}};
        auto it = feeMap.find(cat);
        return it == feeMap.end() ? 50.0 : it->second;
    };

    int regCounter = 0;
    auto addReg = [&](const std::string& patientId, int dayOff, bool done, bool cancelled) -> std::string {
        int dIdx = (regCounter * 3) % 5;          // 科室
        int k = (regCounter / 5) % 4;             // 科室内医生序号
        const Doctor& doc = store.doctors[dIdx * 4 + k];
        ++regCounter;
        std::string date = dateOffset(dayOff);
        Registration r;
        r.id = store.nextId("REG");
        r.patientId = patientId;
        r.doctorId = doc.id;
        r.deptId = doc.deptId;
        r.date = date;
        r.slot = (regCounter % 2 == 0) ? "上午" : "下午";
        r.type = (regCounter % 3 == 0) ? "现场" : "预约";
        r.status = cancelled ? RS_CANCEL : (done ? RS_DONE : RS_WAIT);
        r.seq = (regCounter % 15) + 1;
        r.fee = (k == 0) ? 30.0 : 10.0;
        store.registrations.push_back(r);

        MedicalRecord rec;
        rec.id = store.nextId("REC");
        rec.patientId = patientId;
        rec.doctorId = doc.id;
        rec.deptId = doc.deptId;
        rec.type = RT_REG;
        rec.date = date;
        rec.time = "09:0" + std::to_string(regCounter % 10);
        rec.chief = r.type + "挂号（" + doc.name + " " + r.slot + "）";
        rec.diagnosis = "";
        rec.advice = "";
        rec.linkedId = r.id;
        store.records.push_back(rec);

        BillItem it;
        it.category = "挂号费";
        it.desc = doc.name + "(" + doc.title + ") 挂号";
        it.amount = r.fee;
        mkBill(patientId, date, dayOff > 0 || done, {it});
        return r.id;
    };

    // 近 6 天 + 今天
    for (int dayOff = 6; dayOff >= 1; --dayOff) {
        for (int i = 0; i < 14; ++i) {
            std::string pid = patientIds[(regCounter * 7 + i) % totalPatients];
            addReg(pid, dayOff, true, false);
        }
    }
    // 今天：14 已就诊 + 8 待就诊 + 4 已取消
    for (int i = 0; i < 14; ++i) {
        std::string pid = patientIds[(regCounter * 7 + i) % totalPatients];
        addReg(pid, 0, true, false);
    }
    for (int i = 0; i < 8; ++i) {
        std::string pid = patientIds[(regCounter * 7 + i) % totalPatients];
        addReg(pid, 0, false, false);
    }
    for (int i = 0; i < 4; ++i) {
        std::string pid = patientIds[(regCounter * 7 + i) % totalPatients];
        addReg(pid, 0, false, true);
    }

    // 看诊记录（已就诊的挂号单）
    int consultIdx = 0;
    for (auto& r : store.registrations) {
        if (r.status != RS_DONE) continue;
        MedicalRecord rec;
        rec.id = store.nextId("REC");
        rec.patientId = r.patientId;
        rec.doctorId = r.doctorId;
        rec.deptId = r.deptId;
        rec.type = RT_CONSULT;
        rec.date = r.date;
        rec.time = (r.slot == "上午" ? "10:" : "15:") + padId(consultIdx % 60, 2);
        rec.chief = CHIEFS[consultIdx % 12];
        rec.diagnosis = DIAGS[consultIdx % 12];
        rec.advice = ADVICES[consultIdx % 12];
        rec.linkedId = r.id;
        store.records.push_back(rec);
        ++consultIdx;
    }

    // 检查单 + 检查报告（约 1/3 已就诊者）
    int examIdx = 0;
    for (const auto& r : store.registrations) {
        if (r.status != RS_DONE || examIdx % 3 != 0) { ++examIdx; continue; }
        ++examIdx;
        int dIdx = 0;
        for (int i = 0; i < 5; ++i) if (r.deptId == store.departments[i].id) { dIdx = i; break; }
        std::string cat = EXAM_CATS[dIdx][examIdx % 4];
        Examination e;
        e.id = store.nextId("EX");
        e.patientId = r.patientId;
        e.doctorId = r.doctorId;
        e.deptId = r.deptId;
        e.category = cat;
        e.items = cat + "检查";
        e.date = r.date;
        e.status = "待检查";
        e.fee = examFee(cat);
        store.exams.push_back(e);

        MedicalRecord rec;
        rec.id = store.nextId("REC");
        rec.patientId = r.patientId;
        rec.doctorId = r.doctorId;
        rec.deptId = r.deptId;
        rec.type = RT_EXAM;
        rec.date = r.date;
        rec.time = "11:20";
        rec.chief = "检查申请：" + cat;
        rec.diagnosis = "";
        rec.advice = "";
        rec.linkedId = e.id;
        store.records.push_back(rec);

        BillItem it;
        it.category = "检查费";
        it.desc = cat + " 检查";
        it.amount = e.fee;
        mkBill(r.patientId, r.date, true, {it});

        // 约 2/3 检查出报告
        if (examIdx % 3 != 0) {
            Report rp;
            rp.id = store.nextId("RP");
            rp.examId = e.id;
            rp.result = "各项指标未见明显异常";
            rp.conclusion = "未见异常，建议定期复查";
            rp.date = r.date;
            rp.doctorId = r.doctorId;
            store.reports.push_back(rp);
            e.status = "已完成";
        }
    }

    // 处方（过去：已发药并扣库存；今天：待发药）
    const char* USAGES[] = {"每日2次，每次1片，饭后服用", "每日3次，每次1粒", "每日1次，每次1片，睡前服用",
                            "每日2次，每次10ml", "每日2次，每次1袋", "外用，每日3次"};
    int rxIdx = 0;
    for (const auto& r : store.registrations) {
        if (r.status != RS_DONE || rxIdx % 3 != 0) { ++rxIdx; continue; }
        ++rxIdx;
        bool todayRx = (r.date == dateOffset(0));
        int drugCount = rnd(seed, 1, 2);
        std::vector<PrescriptionItem> items;
        std::vector<int> used;
        for (int c = 0; c < drugCount; ++c) {
            int di = rnd(seed, 0, (int)store.drugs.size() - 1);
            if (std::find(used.begin(), used.end(), di) != used.end()) { --c; continue; }
            used.push_back(di);
            PrescriptionItem it;
            it.drugId = store.drugs[di].id;
            it.qty = rnd(seed, 1, 3);
            it.usage = USAGES[rxIdx % 6];
            items.push_back(it);
        }
        Prescription rx;
        rx.id = store.nextId("RX");
        rx.patientId = r.patientId;
        rx.doctorId = r.doctorId;
        rx.deptId = r.deptId;
        rx.date = r.date;
        rx.status = todayRx ? RX_PENDING : RX_DISPENSED;
        rx.items = items;
        store.prescriptions.push_back(rx);

        double total = 0;
        for (const auto& it : items) {
            for (auto& d : store.drugs) {
                if (d.id == it.drugId) {
                    total += d.price * it.qty;
                    if (rx.status == RX_DISPENSED) {
                        d.stock -= it.qty;
                        StockRecord sr;
                        sr.id = store.nextId("ST");
                        sr.drugId = d.id;
                        sr.type = ST_DISPENSE;
                        sr.date = r.date;
                        sr.operatorName = "药剂师PH00001";
                        sr.remark = "处方发药";
                        sr.refId = rx.id;
                        sr.qty = it.qty;
                        store.stockRecords.push_back(sr);
                    }
                }
            }
        }
        BillItem it;
        it.category = "药费";
        it.desc = "处方 " + rx.id + " 药品费用";
        it.amount = total;
        mkBill(r.patientId, r.date, !todayRx, {it});
    }

    // 制造几个库存预警药品（库存低于下限）
    if (store.drugs.size() >= 5) {
        store.drugs[0].stock = 30;   // 阿莫西林：minStock 60
        store.drugs[3].stock = 18;   // 对乙酰氨基酚：minStock 30
        store.drugs[6].stock = 24;   // 硝苯地平：minStock 30
    }

    // ---------- 住院（32 在院 + 10 已出院） ----------
    int activeCount = 32, dischargedCount = 10;
    int bedIdx = 0;
    auto occupyNextBed = [&](const std::string& patientId) -> std::string {
        while (bedIdx < (int)store.beds.size() && store.beds[bedIdx].status != BS_FREE) ++bedIdx;
        std::string bid = store.beds[bedIdx].id;
        store.beds[bedIdx].status = BS_OCCUPIED;
        store.beds[bedIdx].patientId = patientId;
        ++bedIdx;
        return bid;
    };

    for (int i = 1; i <= activeCount; ++i) {
        std::string pid = "P" + padId(i, 5);
        std::string bid = occupyNextBed(pid);
        const Ward* ward = nullptr;
        for (const auto& w : store.wards) if (w.id == bid.substr(0, w.id.size())) { ward = &w; break; }
        if (!ward) ward = &store.wards[0];
        const Doctor& doc = store.doctors[i % 20];

        Hospitalization h;
        h.id = store.nextId("HOS");
        h.patientId = pid;
        h.bedId = bid;
        h.wardId = ward->id;
        h.doctorId = doc.id;
        h.admitDate = dateOffset(rnd(seed, 1, 6));
        h.status = "住院中";
        store.hospitalizations.push_back(h);

        MedicalRecord rec;
        rec.id = store.nextId("REC");
        rec.patientId = pid;
        rec.doctorId = doc.id;
        rec.deptId = doc.deptId;
        rec.type = RT_HOSP;
        rec.date = h.admitDate;
        rec.time = "14:30";
        rec.chief = "办理入院：" + ward->name + " " + bid;
        rec.diagnosis = DIAGS[(i * 3) % 12];
        rec.advice = "住院观察治疗";
        rec.linkedId = h.id;
        store.records.push_back(rec);

        BillItem it;
        it.category = "住院费";
        it.desc = ward->name + " 住院押金";
        it.amount = 1000;
        mkBill(pid, h.admitDate, false, {it});
    }

    for (int i = 1; i <= dischargedCount; ++i) {
        std::string pid = "P" + padId(activeCount + i, 5);
        const Ward& ward = store.wards[i % store.wards.size()];
        const Doctor& doc = store.doctors[(i + 5) % 20];
        std::string bid = ward.id + "-0" + std::to_string(i % 9 + 1);
        int admitAgo = rnd(seed, 20, 30);
        int stayDays = rnd(seed, 3, 14);

        Hospitalization h;
        h.id = store.nextId("HOS");
        h.patientId = pid;
        h.bedId = bid;
        h.wardId = ward.id;
        h.doctorId = doc.id;
        h.admitDate = dateOffset(admitAgo);
        h.dischargeDate = dateOffset(admitAgo - stayDays);
        h.status = "已出院";
        store.hospitalizations.push_back(h);

        MedicalRecord rec;
        rec.id = store.nextId("REC");
        rec.patientId = pid;
        rec.doctorId = doc.id;
        rec.deptId = doc.deptId;
        rec.type = RT_HOSP;
        rec.date = h.admitDate;
        rec.time = "09:10";
        rec.chief = "办理入院：" + ward.name + " " + bid;
        rec.diagnosis = DIAGS[(i * 5) % 12];
        rec.advice = "出院后遵医嘱，定期复查";
        rec.linkedId = h.id;
        store.records.push_back(rec);

        BillItem d1;
        d1.category = "住院费";
        d1.desc = ward.name + " 住院押金";
        d1.amount = 1000;
        BillItem d2;
        d2.category = "床位费";
        d2.desc = ward.name + " " + std::to_string(stayDays) + " 天 x " + fmtMoney(ward.bedFee);
        d2.amount = ward.bedFee * stayDays;
        mkBill(pid, h.admitDate, true, {d1, d2});
    }

    // ---------- 护理记录（每位在院患者 2 条） ----------
    const char* NURSE_CONTENTS[] = {"生命体征平稳，遵医嘱用药", "体温正常，饮食睡眠可", "伤口敷料干燥，无渗血",
                                    "血压平稳，继续观察", "输液顺利完成，无不良反应", "指导康复锻炼，注意安全"};
    int nrIdx = 0;
    for (const auto& h : store.hospitalizations) {
        if (h.status != "住院中") continue;
        for (int k = 0; k < 2; ++k) {
            NursingRecord nr;
            nr.id = store.nextId("NR");
            nr.patientId = h.patientId;
            nr.bedId = h.bedId;
            nr.nurseId = store.nurses[nrIdx % store.nurses.size()].id;
            nr.date = dateOffset(k);
            nr.time = k == 0 ? "08:00" : "16:00";
            nr.content = NURSE_CONTENTS[nrIdx % 6];
            store.nursingRecords.push_back(nr);
            ++nrIdx;
        }
    }

    msgs.push_back("演示数据已生成：患者 " + std::to_string(store.patients.size()) +
                   " 名、医生 " + std::to_string(store.doctors.size()) + " 名、药品 " +
                   std::to_string(store.drugs.size()) + " 种、床位 " + std::to_string(store.beds.size()) + " 张");
    return true;
}

} // namespace hm
