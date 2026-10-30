#ifndef BUSINESSTYPE_H
#define BUSINESSTYPE_H

#include <QString>

namespace RetailNext {

enum class BusinessType {
    GeneralRetail,
    ClothingFashion,
    Supermarket,
    GroceriesProvisions,
    Footwear,
    Electronics,
    Bakery,
    Stationery,
    Hardware,
    Custom
};

inline QString businessTypeToString(BusinessType type) {
    switch (type) {
    case BusinessType::GeneralRetail:       return QStringLiteral("General Retail");
    case BusinessType::ClothingFashion:     return QStringLiteral("Clothing & Fashion");
    case BusinessType::Supermarket:         return QStringLiteral("Supermarket");
    case BusinessType::GroceriesProvisions: return QStringLiteral("Groceries & Provisions");
    case BusinessType::Footwear:            return QStringLiteral("Footwear");
    case BusinessType::Electronics:         return QStringLiteral("Electronics");
    case BusinessType::Bakery:              return QStringLiteral("Bakery");
    case BusinessType::Stationery:          return QStringLiteral("Stationery");
    case BusinessType::Hardware:            return QStringLiteral("Hardware");
    case BusinessType::Custom:              return QStringLiteral("Custom Retail");
    }
    return QStringLiteral("General Retail");
}

} // namespace RetailNext

#endif // BUSINESSTYPE_H
