#pragma once
// ============================================================
// 医疗管理系统 - 实体类定义（含序列化/反序列化）
// 字段分隔符：| ；子列表分隔符：; ；子字段分隔符：,
// ============================================================
#include <string>
#include <vector>
#include "common.h"

namespace hm {

// ---------- 人员基类 ----------
struct Person {
    std::string id;        // 唯一编号
    std::string name;      // 姓名（允许重名，以 ID 区分）
    std::string gender;    // 男/女
    int age = 0;
    std::string idCard;    // 身份证号
    std::string phone;
    std::string username;  // 登录账号
    std::string password;  // 登录密码
    std::string role;      // 角色
};

// ---------- 患者 ----------
struct Patient : public Person {
    std::string bloodType;   // 血型
    std::string allergy;     // 过敏史
    std::string address;
    std::string regDate;     // 建档日期
    std::string status;      // 门诊/住院
    std::string toLine() const;
    static bool fromLine(const std::string& line, Patient& p);
};

// ---------- 医生 ----------
struct Doctor : public Person {
    std::string deptId;      // 所属科室
    std::string title;       // 职称
    std::string specialty;   // 专长
    std::string toLine() const;
    static bool fromLine(const std::string& line, Doctor& d);
};

// ---------- 护士 ----------
struct Nurse : public Person {
    std::string deptId;
    std::string toLine() const;
    static bool fromLine(const std::string& line, Nurse& n);
};

// ---------- 药剂师 ----------
struct Pharmacist : public Person {
    std::string toLine() const;
    static bool fromLine(const std::string& line, Pharmacist& p);
};

// ---------- 管理员 ----------
struct Admin : public Person {
    std::string toLine() const;
    static bool fromLine(const std::string& line, Admin& a);
};

// ---------- 科室 ----------
struct Department {
    std::string id, name, location, desc;
    std::string wardType;          // 关联病房类型：普通/ICU/隔离/无
    std::vector<std::string> drugIds; // 常用药品（部分药品与科室相关）
    std::string toLine() const;
    static bool fromLine(const std::string& line, Department& d);
};

// ---------- 挂号单 ----------
struct Registration {
    std::string id, patientId, doctorId, deptId;
    std::string date, slot;        // 日期 yyyy-mm-dd，时段 上午/下午
    std::string type;              // 预约/现场
    std::string status;            // 待就诊/已就诊/已取消
    int seq = 0;                   // 号序
    double fee = 0;                // 挂号费
    std::string toLine() const;
    static bool fromLine(const std::string& line, Registration& r);
};

// ---------- 医疗记录（挂号/看诊/检查/住院 四类） ----------
struct MedicalRecord {
    std::string id, patientId, doctorId, deptId;
    std::string type;              // 挂号/看诊/检查/住院
    std::string date, time;
    std::string chief;             // 主诉
    std::string diagnosis;         // 诊断
    std::string advice;            // 医嘱/内容
    std::string linkedId;          // 关联单号（挂号/检查/住院单）
    std::string toLine() const;
    static bool fromLine(const std::string& line, MedicalRecord& r);
};

// ---------- 检查单 ----------
struct Examination {
    std::string id, patientId, doctorId, deptId;
    std::string category;          // 血常规/胸片/心电图/CT...
    std::string items;             // 检查项目
    std::string date;
    std::string status;            // 待检查/已完成
    double fee = 0;
    std::string toLine() const;
    static bool fromLine(const std::string& line, Examination& e);
};

// ---------- 检查报告 ----------
struct Report {
    std::string id, examId;
    std::string result;            // 检查结果
    std::string conclusion;        // 结论
    std::string date;
    std::string doctorId;          // 报告医生
    std::string toLine() const;
    static bool fromLine(const std::string& line, Report& r);
};

// ---------- 处方明细 ----------
struct PrescriptionItem {
    std::string drugId;
    int qty = 0;
    std::string usage;             // 用法用量
    std::string toField() const {
        return drugId + "," + std::to_string(qty) + "," + usage;
    }
    static bool fromField(const std::string& f, PrescriptionItem& it) {
        auto v = split(f, ',');
        if (v.size() < 2) return false;
        it.drugId = v[0];
        it.qty = toInt(v[1]);
        it.usage = v.size() > 2 ? v[2] : "";
        return !it.drugId.empty() && it.qty > 0;
    }
};

// ---------- 处方 ----------
struct Prescription {
    std::string id, patientId, doctorId, deptId;
    std::string date;
    std::string status;            // 待发药/已发药/已取消
    std::vector<PrescriptionItem> items;
    std::string toLine() const;
    static bool fromLine(const std::string& line, Prescription& rx);
};

// ---------- 病房 ----------
struct Ward {
    std::string id, name;
    std::string type;              // 普通/ICU/隔离
    std::string deptId;            // 关联科室（可为空）
    std::string desc;
    double bedFee = 0;             // 床位费/天
    std::string toLine() const;
    static bool fromLine(const std::string& line, Ward& w);
};

// ---------- 床位 ----------
struct Bed {
    std::string id, wardId;
    std::string status;            // 空闲/占用/清洁中/维修中
    std::string patientId;         // 当前占用患者（空闲时为空）
    std::string toLine() const;
    static bool fromLine(const std::string& line, Bed& b);
};

// ---------- 住院单 ----------
struct Hospitalization {
    std::string id, patientId, bedId, wardId, doctorId;
    std::string admitDate, dischargeDate;
    std::string status;            // 住院中/已出院
    std::string remark;
    std::string toLine() const;
    static bool fromLine(const std::string& line, Hospitalization& h);
};

// ---------- 护理记录 ----------
struct NursingRecord {
    std::string id, patientId, bedId, nurseId;
    std::string date, time, content;
    std::string toLine() const;
    static bool fromLine(const std::string& line, NursingRecord& n);
};

// ---------- 药品 ----------
struct Drug {
    std::string id;
    std::string genericName;       // 通用名
    std::string brandName;         // 商品名
    std::string alias;             // 别名（多个用 / 分隔）
    std::string category;          // 分类
    std::string spec;              // 规格
    std::string unit;              // 单位
    std::string deptId;            // 关联科室（可为空）
    std::string manufacturer;      // 生产厂家
    std::string expiry;            // 有效期至
    double price = 0;
    int stock = 0;
    int minStock = 0;              // 库存下限
    std::string toLine() const;
    static bool fromLine(const std::string& line, Drug& d);
};

// ---------- 出入库记录 ----------
struct StockRecord {
    std::string id, drugId;
    std::string type;              // 入库/发药/退货/损耗
    std::string date;
    std::string operatorName;      // 操作人
    std::string remark;
    std::string refId;             // 关联单据（处方/采购单号）
    int qty = 0;
    std::string toLine() const;
    static bool fromLine(const std::string& line, StockRecord& s);
};

// ---------- 账单明细 ----------
struct BillItem {
    std::string category;          // 挂号费/检查费/药费/住院费/床位费/其他
    std::string desc;
    double amount = 0;
    std::string toField() const {
        return category + "," + desc + "," + fmtMoney(amount);
    }
    static bool fromField(const std::string& f, BillItem& it) {
        auto v = split(f, ',');
        if (v.size() < 3) return false;
        it.category = v[0];
        it.desc = v[1];
        it.amount = toDouble(v[2]);
        return !it.category.empty();
    }
};

// ---------- 账单 ----------
struct Bill {
    std::string id, patientId;
    std::string date, status, payDate;
    std::vector<BillItem> items;
    double total() const {
        double s = 0;
        for (const auto& it : items) s += it.amount;
        return s;
    }
    std::string toLine() const;
    static bool fromLine(const std::string& line, Bill& b);
};

} // namespace hm
