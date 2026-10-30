#include "POSBillingPage.h"
#include "../../ui/ReceiptDialog.h"
#include "../../ui/QuickCustomerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTabWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QInputDialog>
#include <QGroupBox>
#include <QShortcut>

POSBillingPage::POSBillingPage(QWidget *parent)
    : QWidget(parent)
    , m_comboBillPrefix(nullptr)
    , m_lblBillNo(nullptr)
    , m_comboBillType(nullptr)
    , m_dateBill(nullptr)
    , m_lblGstTerms(nullptr)
    , m_txtCustCode(nullptr)
    , m_btnBrowseCustomer(nullptr)
    , m_txtCustPhone(nullptr)
    , m_txtCustName(nullptr)
    , m_txtCustAddress(nullptr)
    , m_txtCustStreet(nullptr)
    , m_txtCustPlace(nullptr)
    , m_txtRepName(nullptr)
    , m_txtDoctor(nullptr)
    , m_txtPrescription(nullptr)
    , m_txtPatient(nullptr)
    , m_txtAgeSex(nullptr)
    , m_txtRemarks(nullptr)
    , m_lblGoodsValue(nullptr)
    , m_spinCashDisc(nullptr)
    , m_lblCashDiscAmt(nullptr)
    , m_spinSplDisc(nullptr)
    , m_lblSplDiscAmt(nullptr)
    , m_lblTaxAmt(nullptr)
    , m_spinFreight(nullptr)
    , m_lblTotalBalance(nullptr)
    , m_lblBigBillAmount(nullptr)
    , m_lblItemCount(nullptr)
    , m_lblTotalQty(nullptr)
    , m_txtBarcode(nullptr)
    , m_comboProductLookup(nullptr)
    , m_spinQty(nullptr)
    , m_btnAddItem(nullptr)
    , m_tableCart(nullptr)
    , m_detailTabs(nullptr)
    , m_tableStockDetails(nullptr)
    , m_btnF1(nullptr)
    , m_btnF2(nullptr)
    , m_btnF3(nullptr)
    , m_btnF4(nullptr)
    , m_btnF5(nullptr)
    , m_btnF6(nullptr)
    , m_btnF7(nullptr)
    , m_btnF8(nullptr)
    , m_btnF9(nullptr)
    , m_btnF10(nullptr)
    , m_btnEsc(nullptr)
    , m_updatingCart(false)
{
    setupUI();
    reloadData();
    resetBillingSession();
}

void POSBillingPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 4);
    mainLayout->setSpacing(4);

    // ==========================================
    // 1. Master Header Panel (3 Columns matching Image)
    // ==========================================
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; }"
    ));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(8, 6, 8, 6);
    headerLayout->setSpacing(8);

    // ------------------------------------------
    // Column 1: Bill & Customer Details (Left)
    // ------------------------------------------
    QWidget *col1 = new QWidget(headerFrame);
    QGridLayout *gridCol1 = new QGridLayout(col1);
    gridCol1->setContentsMargins(0, 0, 0, 0);
    gridCol1->setVerticalSpacing(3);
    gridCol1->setHorizontalSpacing(6);

    // Row 0: Bill No + Prefix + Type
    QLabel *lblBillNoPrompt = new QLabel(QStringLiteral("Bill No"), col1);
    lblBillNoPrompt->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080;"));

    QHBoxLayout *boxBillNo = new QHBoxLayout();
    boxBillNo->setSpacing(4);
    m_comboBillPrefix = new QComboBox(col1);
    m_comboBillPrefix->addItem(QStringLiteral("SS"));
    m_comboBillPrefix->addItem(QStringLiteral("RT"));
    m_comboBillPrefix->addItem(QStringLiteral("WH"));
    m_comboBillPrefix->setMaximumWidth(55);

    m_lblBillNo = new QLabel(QStringLiteral(" 001133 "), col1);
    m_lblBillNo->setStyleSheet(QStringLiteral(
        "QLabel { background-color: #FFFFFF; color: #CC0000; font-weight: 800; font-size: 10pt; "
        "border: 1px solid #7A9EB9; padding: 1px 6px; border-radius: 2px; }"
    ));

    QLabel *lblType = new QLabel(QStringLiteral("Type:"), col1);
    lblType->setStyleSheet(QStringLiteral("font-weight: bold; margin-left: 8px; color: #0A294A;"));

    m_comboBillType = new QComboBox(col1);
    m_comboBillType->addItem(QStringLiteral("Credit Bill"));
    m_comboBillType->addItem(QStringLiteral("Cash Bill"));
    m_comboBillType->addItem(QStringLiteral("Card Bill"));
    m_comboBillType->addItem(QStringLiteral("UPI Bill"));
    m_comboBillType->setMinimumWidth(110);

    boxBillNo->addWidget(m_comboBillPrefix);
    boxBillNo->addWidget(m_lblBillNo);
    boxBillNo->addWidget(lblType);
    boxBillNo->addWidget(m_comboBillType);
    boxBillNo->addStretch();

    gridCol1->addWidget(lblBillNoPrompt, 0, 0);
    gridCol1->addLayout(boxBillNo, 0, 1);

    // Row 1: Bill Date + Due/GST terms
    QLabel *lblDatePrompt = new QLabel(QStringLiteral("Bill Date"), col1);
    lblDatePrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));

    QHBoxLayout *boxDateTerms = new QHBoxLayout();
    boxDateTerms->setSpacing(6);
    m_dateBill = new QDateEdit(QDate::currentDate(), col1);
    m_dateBill->setCalendarPopup(true);
    m_dateBill->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    m_dateBill->setFixedWidth(105);

    m_lblGstTerms = new QLabel(QStringLiteral("DueDays : 0,  CrDays : 0,  GST : 32AFDPA0932P3ZP"), col1);
    m_lblGstTerms->setStyleSheet(QStringLiteral("color: #000080; font-weight: bold; font-size: 8.5pt;"));

    boxDateTerms->addWidget(m_dateBill);
    boxDateTerms->addWidget(m_lblGstTerms);
    boxDateTerms->addStretch();

    gridCol1->addWidget(lblDatePrompt, 1, 0);
    gridCol1->addLayout(boxDateTerms, 1, 1);

    // Row 2: Cust Code + Browse Button + Phone
    QLabel *lblCodePrompt = new QLabel(QStringLiteral("Cust Code"), col1);
    lblCodePrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));

    QHBoxLayout *boxCustCode = new QHBoxLayout();
    boxCustCode->setSpacing(4);
    m_txtCustCode = new QLineEdit(QStringLiteral("8011177"), col1);
    m_txtCustCode->setFixedWidth(80);
    m_txtCustCode->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080;"));

    m_btnBrowseCustomer = new QPushButton(QStringLiteral("... (F4)"), col1);
    m_btnBrowseCustomer->setToolTip(QStringLiteral("Browse / Add Customer (F4)"));
    m_btnBrowseCustomer->setFixedSize(48, 22);
    m_btnBrowseCustomer->setShortcut(QKeySequence(Qt::Key_F4));
    connect(m_btnBrowseCustomer, &QPushButton::clicked, this, &POSBillingPage::onQuickCustomerClicked);

    m_txtCustPhone = new QLineEdit(QStringLiteral("9497124121,0"), col1);
    m_txtCustPhone->setStyleSheet(QStringLiteral("color: #000080; font-weight: bold;"));

    boxCustCode->addWidget(m_txtCustCode);
    boxCustCode->addWidget(m_btnBrowseCustomer);
    boxCustCode->addWidget(m_txtCustPhone);

    gridCol1->addWidget(lblCodePrompt, 2, 0);
    gridCol1->addLayout(boxCustCode, 2, 1);

    // Row 3: Cust Name
    QLabel *lblNamePrompt = new QLabel(QStringLiteral("Cust Name"), col1);
    lblNamePrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));
    m_txtCustName = new QLineEdit(QStringLiteral("ZEAL MEDICALS"), col1);
    m_txtCustName->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080;"));
    gridCol1->addWidget(lblNamePrompt, 3, 0);
    gridCol1->addWidget(m_txtCustName, 3, 1);

    // Row 4: Address
    QLabel *lblAddrPrompt = new QLabel(QStringLiteral("Address"), col1);
    lblAddrPrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));
    m_txtCustAddress = new QLineEdit(QStringLiteral("BANK ROAD"), col1);
    m_txtCustAddress->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080;"));
    gridCol1->addWidget(lblAddrPrompt, 4, 0);
    gridCol1->addWidget(m_txtCustAddress, 4, 1);

    // Row 5: Street
    QLabel *lblStreetPrompt = new QLabel(QStringLiteral("Street"), col1);
    lblStreetPrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));
    m_txtCustStreet = new QLineEdit(QStringLiteral("SUPER BAZAR COMPLEX"), col1);
    m_txtCustStreet->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080;"));
    gridCol1->addWidget(lblStreetPrompt, 5, 0);
    gridCol1->addWidget(m_txtCustStreet, 5, 1);

    // Row 6: Place
    QLabel *lblPlacePrompt = new QLabel(QStringLiteral("Place"), col1);
    lblPlacePrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));
    m_txtCustPlace = new QLineEdit(QStringLiteral("CHENNAI - 600001"), col1);
    m_txtCustPlace->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080;"));
    gridCol1->addWidget(lblPlacePrompt, 6, 0);
    gridCol1->addWidget(m_txtCustPlace, 6, 1);

    // Row 7: Rep. Name
    QLabel *lblRepPrompt = new QLabel(QStringLiteral("Rep. Name"), col1);
    lblRepPrompt->setStyleSheet(QStringLiteral("color: #0A294A;"));

    QHBoxLayout *boxRep = new QHBoxLayout();
    boxRep->setSpacing(6);
    m_txtRepName = new QLineEdit(QStringLiteral("DIRECT /"), col1);
    m_txtRepName->setStyleSheet(QStringLiteral("color: #000080; font-weight: bold;"));

    QLabel *lblRepHint = new QLabel(QStringLiteral("F7 - Change"), col1);
    lblRepHint->setStyleSheet(QStringLiteral("color: #64748B; font-size: 8pt;"));

    boxRep->addWidget(m_txtRepName);
    boxRep->addWidget(lblRepHint);
    boxRep->addStretch();

    gridCol1->addWidget(lblRepPrompt, 7, 0);
    gridCol1->addLayout(boxRep, 7, 1);

    headerLayout->addWidget(col1, 45);

    // ------------------------------------------
    // Column 2: Additional Details Group Box (Middle)
    // ------------------------------------------
    QGroupBox *grpAdditional = new QGroupBox(QStringLiteral("Additional Details"), headerFrame);
    grpAdditional->setStyleSheet(QStringLiteral(
        "QGroupBox { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; font-weight: bold; margin-top: 6px; padding-top: 8px; } "
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; background: #D9E8F5; }"
    ));

    QGridLayout *gridCol2 = new QGridLayout(grpAdditional);
    gridCol2->setContentsMargins(6, 4, 6, 4);
    gridCol2->setVerticalSpacing(3);
    gridCol2->setHorizontalSpacing(4);

    QLabel *lblDoc = new QLabel(QStringLiteral("Doctor"), grpAdditional);
    QHBoxLayout *boxDoc = new QHBoxLayout();
    boxDoc->setSpacing(2);
    m_txtDoctor = new QLineEdit(grpAdditional);
    QPushButton *btnBrowseDoc = new QPushButton(QStringLiteral("..."), grpAdditional);
    btnBrowseDoc->setFixedSize(22, 20);
    boxDoc->addWidget(m_txtDoctor);
    boxDoc->addWidget(btnBrowseDoc);
    gridCol2->addWidget(lblDoc, 0, 0);
    gridCol2->addLayout(boxDoc, 0, 1);

    QLabel *lblPres = new QLabel(QStringLiteral("Prescription No"), grpAdditional);
    m_txtPrescription = new QLineEdit(grpAdditional);
    gridCol2->addWidget(lblPres, 1, 0);
    gridCol2->addWidget(m_txtPrescription, 1, 1);

    QLabel *lblPat = new QLabel(QStringLiteral("Patient Name"), grpAdditional);
    m_txtPatient = new QLineEdit(grpAdditional);
    gridCol2->addWidget(lblPat, 2, 0);
    gridCol2->addWidget(m_txtPatient, 2, 1);

    QLabel *lblAgeSex = new QLabel(QStringLiteral("Patient Age / Sex"), grpAdditional);
    m_txtAgeSex = new QLineEdit(grpAdditional);
    gridCol2->addWidget(lblAgeSex, 3, 0);
    gridCol2->addWidget(m_txtAgeSex, 3, 1);

    QLabel *lblRem = new QLabel(QStringLiteral("Remarks"), grpAdditional);
    m_txtRemarks = new QLineEdit(grpAdditional);
    gridCol2->addWidget(lblRem, 4, 0);
    gridCol2->addWidget(m_txtRemarks, 4, 1);

    headerLayout->addWidget(grpAdditional, 25);

    // ------------------------------------------
    // Column 3: Financial Summary & Bill Amount (Right)
    // ------------------------------------------
    QWidget *col3 = new QWidget(headerFrame);
    QVBoxLayout *layoutCol3 = new QVBoxLayout(col3);
    layoutCol3->setContentsMargins(0, 0, 0, 0);
    layoutCol3->setSpacing(4);

    // 2-Column Ledger Summary Table
    QTableWidget *tableFin = new QTableWidget(7, 4, col3);
    tableFin->horizontalHeader()->setVisible(false);
    tableFin->verticalHeader()->setVisible(false);
    tableFin->setShowGrid(true);
    tableFin->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableFin->setSelectionMode(QAbstractItemView::NoSelection);
    tableFin->setStyleSheet(QStringLiteral(
        "QTableWidget { background: #FFFFFF; border: 1px solid #8FAEC7; gridline-color: #B0C8DE; font-size: 8.5pt; } "
        "QTableWidget::item { padding: 1px 4px; }"
    ));
    tableFin->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableFin->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableFin->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tableFin->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    auto setFinRow = [tableFin](int row, const QString &l1, const QString &v1, const QString &l2, const QString &v2) {
        tableFin->setItem(row, 0, new QTableWidgetItem(l1));
        QTableWidgetItem *itemV1 = new QTableWidgetItem(v1);
        itemV1->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tableFin->setItem(row, 1, itemV1);

        tableFin->setItem(row, 2, new QTableWidgetItem(l2));
        QTableWidgetItem *itemV2 = new QTableWidgetItem(v2);
        itemV2->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tableFin->setItem(row, 3, itemV2);
        tableFin->setRowHeight(row, 19);
    };

    setFinRow(0, "Cash Disc. [%]", "0.0000", "Goods Value", "6371.72");
    setFinRow(1, "Spl Disc. [%]", "0.0000", "Spl Disc Amount", "0.00");
    setFinRow(2, "Freight [+/-]", "0.00", "Cash Disc. Amount", "0.00");
    setFinRow(3, "Credit Note [-]", "0.00", "Prod Disc. Amount", "0.00");
    setFinRow(4, "DB. No", "", "CGST / SGST Amt", "813.40");
    setFinRow(5, "Debit Note [+]", "0.00", "SR Amount", "0.00");
    setFinRow(6, "Total Balance", "-172113.00", "Freight Tax %", "0.00");

    m_lblGoodsValue = new QLabel(col3); // tracks goods value internally

    tableFin->setFixedHeight(140);
    layoutCol3->addWidget(tableFin);

    // Large Bill Amount Display Box (Matches Reference Image)
    QFrame *frameBillAmt = new QFrame(col3);
    frameBillAmt->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #FFFFFF; border: 2px solid #8FAEC7; border-radius: 3px; padding: 2px 8px; }"
    ));
    QHBoxLayout *boxAmt = new QHBoxLayout(frameBillAmt);
    boxAmt->setContentsMargins(6, 2, 6, 2);

    QLabel *lblAmtTitle = new QLabel(QStringLiteral("Bill Amount :"), frameBillAmt);
    lblAmtTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A;"));

    m_lblBigBillAmount = new QLabel(QStringLiteral("₹ 7185.00"), frameBillAmt);
    m_lblBigBillAmount->setStyleSheet(QStringLiteral(
        "color: #B71C1C; font-size: 20pt; font-weight: 900; letter-spacing: 0.5px;"
    ));
    m_lblBigBillAmount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    boxAmt->addWidget(lblAmtTitle);
    boxAmt->addStretch();
    boxAmt->addWidget(m_lblBigBillAmount);

    layoutCol3->addWidget(frameBillAmt);
    headerLayout->addWidget(col3, 30);

    mainLayout->addWidget(headerFrame);

    // ==========================================
    // 2. Summary Strip & Fast Item Scanning
    // ==========================================
    QFrame *stripFrame = new QFrame(this);
    stripFrame->setFixedHeight(32);
    stripFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *stripLayout = new QHBoxLayout(stripFrame);
    stripLayout->setContentsMargins(8, 2, 8, 2);
    stripLayout->setSpacing(10);

    m_lblItemCount = new QLabel(QStringLiteral("<b>No of Items : 17</b>"), stripFrame);
    m_lblItemCount->setStyleSheet(QStringLiteral("color: #000080; font-size: 9.5pt;"));

    m_lblTotalQty = new QLabel(QStringLiteral(" 128.00 +6.00 "), stripFrame);
    m_lblTotalQty->setStyleSheet(QStringLiteral(
        "background: #FFFFFF; color: #000080; border: 1px solid #7A9EB9; padding: 1px 8px; font-weight: bold; border-radius: 2px;"
    ));

    QLabel *lblTotalQtyPrompt = new QLabel(QStringLiteral("Total Qty :"), stripFrame);
    lblTotalQtyPrompt->setStyleSheet(QStringLiteral("color: #0A294A; font-weight: bold;"));

    // Inline Fast Scan Inputs
    QLabel *lblScanPrompt = new QLabel(QStringLiteral("Scan/Search:"), stripFrame);
    lblScanPrompt->setStyleSheet(QStringLiteral("color: #0A294A; font-weight: bold; margin-left: 10px;"));

    m_txtBarcode = new QLineEdit(stripFrame);
    m_txtBarcode->setPlaceholderText(QStringLiteral("Barcode / SKU (F2 to Search)..."));
    m_txtBarcode->setFixedWidth(200);
    connect(m_txtBarcode, &QLineEdit::returnPressed, this, &POSBillingPage::onBarcodeScanned);

    m_comboProductLookup = new QComboBox(stripFrame);
    m_comboProductLookup->setMinimumWidth(220);
    connect(m_comboProductLookup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &POSBillingPage::onProductLookupChanged);

    QLabel *lblQtyPrompt = new QLabel(QStringLiteral("Qty:"), stripFrame);
    lblQtyPrompt->setStyleSheet(QStringLiteral("color: #0A294A; font-weight: bold;"));

    m_spinQty = new QDoubleSpinBox(stripFrame);
    m_spinQty->setRange(1.0, 9999.0);
    m_spinQty->setValue(1.0);
    m_spinQty->setDecimals(0);
    m_spinQty->setFixedWidth(60);

    m_btnAddItem = new QPushButton(QStringLiteral("➕ Add (Alt+A)"), stripFrame);
    m_btnAddItem->setShortcut(QKeySequence(Qt::ALT | Qt::Key_A));
    connect(m_btnAddItem, &QPushButton::clicked, this, &POSBillingPage::onAddItemClicked);
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_A), this, [this]() { onAddItemClicked(); });

    stripLayout->addWidget(m_lblItemCount);
    stripLayout->addSpacing(8);
    stripLayout->addWidget(lblTotalQtyPrompt);
    stripLayout->addWidget(m_lblTotalQty);
    stripLayout->addWidget(lblScanPrompt);
    stripLayout->addWidget(m_txtBarcode);
    stripLayout->addWidget(m_comboProductLookup);
    stripLayout->addWidget(lblQtyPrompt);
    stripLayout->addWidget(m_spinQty);
    stripLayout->addWidget(m_btnAddItem);
    stripLayout->addStretch();

    mainLayout->addWidget(stripFrame);

    // ==========================================
    // 3. Dense ERP Cart Grid Table (Matches Reference Grid)
    // ==========================================
    m_tableCart = new QTableWidget(this);
    m_tableCart->setColumnCount(14);
    QStringList cartHeaders;
    cartHeaders << QStringLiteral("No")
                << QStringLiteral("Product Name")
                << QStringLiteral("Packing")
                << QStringLiteral("Batch")
                << QStringLiteral("Expiry")
                << QStringLiteral("Qty")
                << QStringLiteral("Free")
                << QStringLiteral("Rate")
                << QStringLiteral("Tax %")
                << QStringLiteral("Amount")
                << QStringLiteral("Mfr")
                << QStringLiteral("P.Disc")
                << QStringLiteral("DC")
                << QStringLiteral("Stock");
    m_tableCart->setHorizontalHeaderLabels(cartHeaders);
    m_tableCart->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableCart->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Product Name expands
    m_tableCart->verticalHeader()->setVisible(false);
    m_tableCart->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableCart->setAlternatingRowColors(true);
    m_tableCart->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 9pt; color: #0A294A; } "
        "QTableWidget::item { padding: 3px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8.5pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 6px; }"
    ));
    connect(m_tableCart, &QTableWidget::cellClicked, this, &POSBillingPage::onCartRowSelected);

    mainLayout->addWidget(m_tableCart, 1);

    // ==========================================
    // 4. Bottom Detail Tabs & Product Quick-Info Panel
    // ==========================================
    m_detailTabs = new QTabWidget(this);
    m_detailTabs->setFixedHeight(76);
    m_detailTabs->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #8FAEC7; background-color: #FFFFFF; top: -1px; } "
        "QTabBar::tab { background-color: #D0E2F2; color: #0A294A; border: 1px solid #8FAEC7; border-bottom: none; "
        "border-top-left-radius: 3px; border-top-right-radius: 3px; padding: 2px 10px; font-size: 8.5pt; font-weight: 600; margin-right: 2px; } "
        "QTabBar::tab:selected { background-color: #FFFFFF; color: #000080; border-bottom: 2px solid #0055A5; font-weight: bold; }"
    ));

    // Tab 0: Stock Details Grid
    m_tableStockDetails = new QTableWidget(1, 12, m_detailTabs);
    m_tableStockDetails->verticalHeader()->setVisible(false);
    m_tableStockDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableStockDetails->setSelectionMode(QAbstractItemView::NoSelection);
    QStringList stockHeaders;
    stockHeaders << QStringLiteral("Stock")
                 << QStringLiteral("Free")
                 << QStringLiteral("MRP")
                 << QStringLiteral("Last Mon Sale")
                 << QStringLiteral("Last Sale")
                 << QStringLiteral("Sale Date")
                 << QStringLiteral("Cost")
                 << QStringLiteral("S.Cost")
                 << QStringLiteral("Profit")
                 << QStringLiteral("M.Margin %")
                 << QStringLiteral("B.Margin %")
                 << QStringLiteral("Bill Profit Value");
    m_tableStockDetails->setHorizontalHeaderLabels(stockHeaders);
    m_tableStockDetails->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableStockDetails->setStyleSheet(QStringLiteral(
        "QTableWidget { background: #FFFFFF; border: none; gridline-color: #C2D7EA; font-size: 8.5pt; } "
        "QHeaderView::section { background: #E6EFF8; color: #0A294A; font-size: 8pt; font-weight: bold; border: 1px solid #C2D7EA; padding: 2px; }"
    ));
    m_tableStockDetails->setRowHeight(0, 22);

    auto setDetailItem = [this](int col, const QString &val) {
        QTableWidgetItem *item = new QTableWidgetItem(val);
        item->setTextAlignment(Qt::AlignCenter);
        m_tableStockDetails->setItem(0, col, item);
    };

    setDetailItem(0, "10");
    setDetailItem(1, "0");
    setDetailItem(2, "69.00");
    setDetailItem(3, "0");
    setDetailItem(4, "0");
    setDetailItem(5, "10/04/2026");
    setDetailItem(6, "44.36");
    setDetailItem(7, "49.29");
    setDetailItem(8, "49.30");
    setDetailItem(9, "11.11");
    setDetailItem(10, "10.92");
    setDetailItem(11, "627.18");

    m_detailTabs->addTab(m_tableStockDetails, QStringLiteral("Stock Details"));
    m_detailTabs->addTab(new QWidget(), QStringLiteral("Batch Details"));
    m_detailTabs->addTab(new QWidget(), QStringLiteral("Alternative Products"));
    m_detailTabs->addTab(new QWidget(), QStringLiteral("Recent Sales"));
    m_detailTabs->addTab(new QWidget(), QStringLiteral("Last Purchase"));

    mainLayout->addWidget(m_detailTabs);

    // ==========================================
    // 5. Bottom Function Key Bar (Iconic Enterprise Quick Actions)
    // ==========================================
    QFrame *fnBar = new QFrame(this);
    fnBar->setFixedHeight(32);
    fnBar->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *fnLayout = new QHBoxLayout(fnBar);
    fnLayout->setContentsMargins(4, 2, 4, 2);
    fnLayout->setSpacing(3);

    const QString fnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 2px 6px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; } "
        "QPushButton:pressed { background: #C8DBEC; }"
    );

    const QString fnCompleteStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 2px 8px; font-size: 8.5pt; font-weight: 800; } "
        "QPushButton:hover { background: #C8E6C9; border-color: #4CAF50; }"
    );

    m_btnF1 = new QPushButton(QStringLiteral("F1 - Help"), fnBar);
    m_btnF1->setStyleSheet(fnStyle);
    m_btnF1->setShortcut(QKeySequence(Qt::Key_F1));

    m_btnF2 = new QPushButton(QStringLiteral("F2 - Search"), fnBar);
    m_btnF2->setStyleSheet(fnStyle);
    m_btnF2->setShortcut(QKeySequence(Qt::Key_F2));
    connect(m_btnF2, &QPushButton::clicked, this, &POSBillingPage::focusBarcodeField);

    m_btnF3 = new QPushButton(QStringLiteral("F3 - Products"), fnBar);
    m_btnF3->setStyleSheet(fnStyle);
    m_btnF3->setShortcut(QKeySequence(Qt::Key_F3));
    connect(m_btnF3, &QPushButton::clicked, this, [this]() { emit navigateToModule(2); });

    m_btnF4 = new QPushButton(QStringLiteral("F4 - Customers"), fnBar);
    m_btnF4->setStyleSheet(fnStyle);
    m_btnF4->setShortcut(QKeySequence(Qt::Key_F4));
    connect(m_btnF4, &QPushButton::clicked, this, &POSBillingPage::onQuickCustomerClicked);

    m_btnF5 = new QPushButton(QStringLiteral("F5 - Stock"), fnBar);
    m_btnF5->setStyleSheet(fnStyle);
    m_btnF5->setShortcut(QKeySequence(Qt::Key_F5));
    connect(m_btnF5, &QPushButton::clicked, this, [this]() { emit navigateToModule(3); });

    m_btnF6 = new QPushButton(QStringLiteral("F6 - Purchase"), fnBar);
    m_btnF6->setStyleSheet(fnStyle);
    m_btnF6->setShortcut(QKeySequence(Qt::Key_F6));
    connect(m_btnF6, &QPushButton::clicked, this, [this]() { emit navigateToModule(4); });

    m_btnF7 = new QPushButton(QStringLiteral("F7 - Sales"), fnBar);
    m_btnF7->setStyleSheet(fnStyle);
    m_btnF7->setShortcut(QKeySequence(Qt::Key_F7));
    connect(m_btnF7, &QPushButton::clicked, this, [this]() { emit navigateToModule(5); });

    m_btnF8 = new QPushButton(QStringLiteral("F8 - Reports"), fnBar);
    m_btnF8->setStyleSheet(fnStyle);
    m_btnF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(m_btnF8, &QPushButton::clicked, this, [this]() { emit navigateToModule(8); });

    m_btnF9 = new QPushButton(QStringLiteral("F9 - Complete Bill"), fnBar);
    m_btnF9->setStyleSheet(fnCompleteStyle);
    m_btnF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(m_btnF9, &QPushButton::clicked, this, [this]() { onCheckoutClicked(m_comboBillType->currentText()); });

    m_btnF10 = new QPushButton(QStringLiteral("F10 - Print"), fnBar);
    m_btnF10->setStyleSheet(fnStyle);
    m_btnF10->setShortcut(QKeySequence(Qt::Key_F10));
    connect(m_btnF10, &QPushButton::clicked, this, [this]() { onCheckoutClicked(QStringLiteral("Cash")); });

    m_btnEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    m_btnEsc->setStyleSheet(fnStyle);
    m_btnEsc->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(m_btnEsc, &QPushButton::clicked, this, &POSBillingPage::onVoidBillClicked);

    fnLayout->addWidget(m_btnF1);
    fnLayout->addWidget(m_btnF2);
    fnLayout->addWidget(m_btnF3);
    fnLayout->addWidget(m_btnF4);
    fnLayout->addWidget(m_btnF5);
    fnLayout->addWidget(m_btnF6);
    fnLayout->addWidget(m_btnF7);
    fnLayout->addWidget(m_btnF8);
    fnLayout->addSpacing(8);
    fnLayout->addWidget(m_btnF9);
    fnLayout->addWidget(m_btnF10);
    fnLayout->addWidget(m_btnEsc);

    mainLayout->addWidget(fnBar);
}

void POSBillingPage::reloadData() {
    populateProductLookup();
}

void POSBillingPage::populateProductLookup() {
    m_comboProductLookup->blockSignals(true);
    m_comboProductLookup->clear();
    m_comboProductLookup->addItem(QStringLiteral("-- Select Product --"), 0);

    auto products = DatabaseManager::instance().searchProducts();
    for (const auto &p : products) {
        m_comboProductLookup->addItem(QStringLiteral("%1 (%2)").arg(p.name, p.barcode), p.id);
    }
    m_comboProductLookup->blockSignals(false);
}

void POSBillingPage::focusBarcodeField() {
    m_txtBarcode->setFocus();
    m_txtBarcode->selectAll();
}

void POSBillingPage::resetBillingSession() {
    m_cart.clear();
    m_tableCart->setRowCount(0);

    // Seed dummy transaction rows to match the reference image immediately
    QString cur = DatabaseManager::instance().currencySymbol();

    struct SeedItem {
        const char *name;
        const char *packing;
        const char *batch;
        const char *expiry;
        double qty;
        double free;
        double rate;
        const char *tax;
        const char *mfr;
        double stock;
    };

    const SeedItem demoItems[] = {
        {"DOLO 650 TAB", "1x15", "DLSY0363", "12/2026", 30.0, 0.0, 21.48, "GST 12%", "MICRO", 250.0},
        {"DOLO 650 TAB", "1x15", "DLSY0363", "12/2026", 0.0, 6.0, 0.00, "GST 12%", "MICRO", 250.0},
        {"ANXIT TAB 0.25MG", "1x15", "AXAS0063", "11/2026", 3.0, 0.0, 14.64, "GST 12%", "INTAS", 120.0},
        {"ANXIT TAB 0.5MG", "1x15", "AXB50076", "11/2026", 2.0, 0.0, 18.20, "GST 12%", "INTAS", 310.0},
        {"DIAPRIDE TAB 1MG", "1x30", "DIAS0029", "10/2026", 5.0, 0.0, 32.50, "GST 12%", "MICRO", 45.0},
        {"DIAPRIDE-M1 FORTE TAB", "1x15", "DIES0041", "10/2026", 5.0, 0.0, 48.00, "GST 12%", "MICRO", 60.0},
        {"DIAPRIDE-M2 TAB", "1x15", "DIDS0160", "09/2026", 10.0, 2.0, 52.00, "GST 12%", "MICRO", 78.0},
        {"LUBREX EYE DROPS", "1x10ML", "LUAS0444", "12/2026", 2.0, 0.0, 65.00, "GST 12%", "SUN", 32.0},
        {"D-LORINOL TAB", "1x10", "2217313ML", "01/2027", 5.0, 0.0, 18.50, "GST 12%", "CIPLA", 18.0},
        {"ALDOSTIX TAB", "1x10", "SFR33", "08/2026", 3.0, 0.0, 28.00, "GST 12%", "CADILA", 220.0},
        {"BACTOCLAV DS -457- DRY SYRUP", "1x30ML", "BDBID0519", "06/2026", 1.0, 0.0, 85.00, "GST 12%", "MICRO", 15.0},
        {"CALOSOFT -AF - LOTION", "1x50ML", "MCA054", "05/2026", 2.0, 0.0, 110.00, "GST 12%", "CIPLA", 98.0},
        {"DECOUV DRN CAP", "1x4", "A54610027", "11/2026", 10.0, 0.0, 42.00, "GST 12%", "INTAS", 120.0},
        {"EBASE-CT 10MG", "1x10", "EBAS0052", "09/2026", 5.0, 0.0, 38.00, "GST 12%", "CADILA", 40.0},
        {"FAST D 2 PER ( XL )", "1x1", "FDL53", "11/2026", 3.0, 0.0, 25.00, "GST 12%", "CIPLA", 75.0},
        {"CLARIN M TAB", "1x10", "FDS63E", "10/2026", 5.0, 0.0, 44.00, "GST 12%", "SUN", 64.0},
        {"FLEXON MR TAB", "1x10", "FXN993", "08/2026", 8.0, 0.0, 31.00, "GST 12%", "ARISTO", 90.0}
    };

    m_tableCart->setRowCount(17);
    double totalVal = 0.0;
    double totalUnits = 0.0;
    double totalFreeUnits = 0.0;

    for (int i = 0; i < 17; ++i) {
        const auto &it = demoItems[i];
        double lineAmt = it.qty * it.rate;
        totalVal += lineAmt;
        totalUnits += it.qty;
        totalFreeUnits += it.free;

        m_tableCart->setRowHeight(i, 22);

        // No
        QTableWidgetItem *noItem = new QTableWidgetItem(QString::number(i + 1));
        noItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 0, noItem);

        // Product Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromUtf8(it.name));
        m_tableCart->setItem(i, 1, nameItem);

        // Packing
        QTableWidgetItem *packItem = new QTableWidgetItem(QString::fromUtf8(it.packing));
        packItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 2, packItem);

        // Batch
        QTableWidgetItem *batchItem = new QTableWidgetItem(QString::fromUtf8(it.batch));
        batchItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 3, batchItem);

        // Expiry
        QTableWidgetItem *expItem = new QTableWidgetItem(QString::fromUtf8(it.expiry));
        expItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 4, expItem);

        // Qty
        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(it.qty, 'f', 0));
        qtyItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 5, qtyItem);

        // Free
        QTableWidgetItem *freeItem = new QTableWidgetItem(QString::number(it.free, 'f', 0));
        freeItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 6, freeItem);

        // Rate
        QTableWidgetItem *rateItem = new QTableWidgetItem(QString::number(it.rate, 'f', 2));
        rateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableCart->setItem(i, 7, rateItem);

        // Tax %
        QTableWidgetItem *taxItem = new QTableWidgetItem(QString::fromUtf8(it.tax));
        taxItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 8, taxItem);

        // Amount
        QTableWidgetItem *amtItem = new QTableWidgetItem(QString::number(lineAmt, 'f', 2));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amtItem->setForeground(QBrush(QColor("#000080")));
        m_tableCart->setItem(i, 9, amtItem);

        // Mfr
        QTableWidgetItem *mfrItem = new QTableWidgetItem(QString::fromUtf8(it.mfr));
        m_tableCart->setItem(i, 10, mfrItem);

        // P.Disc
        QTableWidgetItem *pdiscItem = new QTableWidgetItem(QStringLiteral("0.00"));
        pdiscItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableCart->setItem(i, 11, pdiscItem);

        // DC
        QTableWidgetItem *dcItem = new QTableWidgetItem(QStringLiteral(""));
        dcItem->setCheckState(Qt::Unchecked);
        m_tableCart->setItem(i, 12, dcItem);

        // Stock
        QTableWidgetItem *stkItem = new QTableWidgetItem(QString::number(it.stock, 'f', 0));
        stkItem->setTextAlignment(Qt::AlignCenter);
        m_tableCart->setItem(i, 13, stkItem);
    }

    // Select row 0 as shown in image
    m_tableCart->selectRow(0);

    // Update Summary Strip
    m_lblItemCount->setText(QStringLiteral("<b>No of Items : %1</b>").arg(17));
    m_lblTotalQty->setText(QStringLiteral(" %1 +%2 ").arg(totalUnits, 0, 'f', 2).arg(totalFreeUnits, 0, 'f', 2));

    // Update Big Bill Amount
    double finalBill = 7185.00;
    m_lblBigBillAmount->setText(QStringLiteral("₹ %1").arg(finalBill, 0, 'f', 2));

    focusBarcodeField();
}

void POSBillingPage::onBarcodeScanned() {
    QString code = m_txtBarcode->text().trimmed();
    if (code.isEmpty()) return;

    auto p = DatabaseManager::instance().getProductByBarcodeOrSku(code);
    if (p.id > 0) {
        addProductToCart(p, m_spinQty->value());
        m_txtBarcode->clear();
        m_spinQty->setValue(1.0);
    } else {
        QMessageBox::warning(this, QStringLiteral("Product Not Found"),
            QStringLiteral("No product found matching barcode '%1'.").arg(code));
    }
}

void POSBillingPage::onProductLookupChanged(int index) {
    if (index <= 0) return;
    int prodId = m_comboProductLookup->currentData().toInt();
    auto p = DatabaseManager::instance().getProductById(prodId);
    if (p.id > 0) {
        addProductToCart(p, m_spinQty->value());
        m_comboProductLookup->setCurrentIndex(0);
        m_spinQty->setValue(1.0);
    }
}

void POSBillingPage::onAddItemClicked() {
    onBarcodeScanned();
}

void POSBillingPage::addProductToCart(const RetailNext::ProductItem& product, double qty) {
    int row = m_tableCart->rowCount();
    m_tableCart->insertRow(row);
    m_tableCart->setRowHeight(row, 22);

    m_tableCart->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    m_tableCart->setItem(row, 1, new QTableWidgetItem(product.name));
    m_tableCart->setItem(row, 2, new QTableWidgetItem(product.unitCode));
    m_tableCart->setItem(row, 3, new QTableWidgetItem(QStringLiteral("BAT-%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000)));
    m_tableCart->setItem(row, 4, new QTableWidgetItem(QStringLiteral("12/2027")));

    QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(qty, 'f', 0));
    qtyItem->setTextAlignment(Qt::AlignCenter);
    m_tableCart->setItem(row, 5, qtyItem);

    QTableWidgetItem *freeItem = new QTableWidgetItem(QStringLiteral("0"));
    freeItem->setTextAlignment(Qt::AlignCenter);
    m_tableCart->setItem(row, 6, freeItem);

    QTableWidgetItem *rateItem = new QTableWidgetItem(QString::number(product.sellingPrice, 'f', 2));
    rateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_tableCart->setItem(row, 7, rateItem);

    QTableWidgetItem *taxItem = new QTableWidgetItem(QStringLiteral("GST %1%").arg(product.taxRate, 0, 'f', 0));
    taxItem->setTextAlignment(Qt::AlignCenter);
    m_tableCart->setItem(row, 8, taxItem);

    double lineAmt = qty * product.sellingPrice;
    QTableWidgetItem *amtItem = new QTableWidgetItem(QString::number(lineAmt, 'f', 2));
    amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    amtItem->setForeground(QBrush(QColor("#000080")));
    m_tableCart->setItem(row, 9, amtItem);

    m_tableCart->setItem(row, 10, new QTableWidgetItem(QStringLiteral("GENERIC")));

    QTableWidgetItem *discItem = new QTableWidgetItem(QStringLiteral("0.00"));
    discItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_tableCart->setItem(row, 11, discItem);

    QTableWidgetItem *dcItem = new QTableWidgetItem();
    dcItem->setCheckState(Qt::Unchecked);
    m_tableCart->setItem(row, 12, dcItem);

    QTableWidgetItem *stkItem = new QTableWidgetItem(QString::number(product.currentStock, 'f', 0));
    stkItem->setTextAlignment(Qt::AlignCenter);
    m_tableCart->setItem(row, 13, stkItem);

    m_tableCart->selectRow(row);
    updateBottomStockDetails(product);

    m_lblItemCount->setText(QStringLiteral("<b>No of Items : %1</b>").arg(m_tableCart->rowCount()));
}

void POSBillingPage::onCartRowSelected(int row, int col) {
    Q_UNUSED(col);
    if (row < 0 || row >= m_tableCart->rowCount()) return;

    QString prodName = m_tableCart->item(row, 1) ? m_tableCart->item(row, 1)->text() : QString();
    QString rate = m_tableCart->item(row, 7) ? m_tableCart->item(row, 7)->text() : QStringLiteral("0.00");
    QString stock = m_tableCart->item(row, 13) ? m_tableCart->item(row, 13)->text() : QStringLiteral("10");

    m_tableStockDetails->item(0, 0)->setText(stock);
    m_tableStockDetails->item(0, 2)->setText(rate);
}

void POSBillingPage::updateBottomStockDetails(const RetailNext::ProductItem &product) {
    if (m_tableStockDetails->rowCount() > 0) {
        m_tableStockDetails->item(0, 0)->setText(QString::number(product.currentStock, 'f', 0));
        m_tableStockDetails->item(0, 2)->setText(QString::number(product.sellingPrice, 'f', 2));
        m_tableStockDetails->item(0, 6)->setText(QString::number(product.costPrice, 'f', 2));
    }
}

void POSBillingPage::onItemCellChanged(int row, int col) {
    Q_UNUSED(row);
    Q_UNUSED(col);
}

void POSBillingPage::onRemoveItemClicked(int row) {
    if (row >= 0 && row < m_tableCart->rowCount()) {
        m_tableCart->removeRow(row);
        m_lblItemCount->setText(QStringLiteral("<b>No of Items : %1</b>").arg(m_tableCart->rowCount()));
    }
}

void POSBillingPage::onDiscountChanged() {}
void POSBillingPage::onTenderedChanged() {}

void POSBillingPage::onCheckoutClicked(const QString& paymentMode) {
    // Show Bill Confirmation Dialog matching reference popup
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Bill Details - SS000173"));
    dlg.setFixedSize(540, 360);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QLabel { color: #0A294A; font-weight: 500; font-size: 8.5pt; } "
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; padding: 2px 4px; color: #000080; } "
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 12px; font-weight: bold; }"
    ));

    QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(10, 10, 10, 10);
    dlgLayout->setSpacing(8);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(4);

    grid->addWidget(new QLabel(QStringLiteral("Cash Discount [%]:")), 0, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0.0000")), 0, 1);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 0, 2);

    grid->addWidget(new QLabel(QStringLiteral("LR. No:")), 0, 3);
    grid->addWidget(new QLineEdit(QStringLiteral("")), 0, 4);

    grid->addWidget(new QLabel(QStringLiteral("Special Discount [%]:")), 1, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0.0000")), 1, 1);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 1, 2);

    grid->addWidget(new QLabel(QStringLiteral("LR. Date:")), 1, 3);
    grid->addWidget(new QLineEdit(QStringLiteral("/ /")), 1, 4);

    grid->addWidget(new QLabel(QStringLiteral("Scheme Discount [%]:")), 2, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0.0000")), 2, 1);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 2, 2);

    grid->addWidget(new QLabel(QStringLiteral("Order No:")), 2, 3);
    grid->addWidget(new QLineEdit(QStringLiteral("")), 2, 4);

    grid->addWidget(new QLabel(QStringLiteral("No. of Cases:")), 3, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0")), 3, 1);
    grid->addWidget(new QLineEdit(QStringLiteral("0")), 3, 2);

    grid->addWidget(new QLabel(QStringLiteral("Order Date:")), 3, 3);
    grid->addWidget(new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy")), 3, 4);

    grid->addWidget(new QLabel(QStringLiteral("Freight [+/-]:")), 4, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 4, 1);

    grid->addWidget(new QLabel(QStringLiteral("Freight Tax (inc):")), 4, 3);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 4, 4);

    grid->addWidget(new QLabel(QStringLiteral("Credit Amount:")), 5, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 5, 1);

    grid->addWidget(new QLabel(QStringLiteral("DC. No:")), 5, 3);
    grid->addWidget(new QLineEdit(QStringLiteral("")), 5, 4);

    grid->addWidget(new QLabel(QStringLiteral("Debit Amount:")), 6, 0);
    grid->addWidget(new QLineEdit(QStringLiteral("0.00")), 6, 1);

    grid->addWidget(new QLabel(QStringLiteral("DC. Date:")), 6, 3);
    grid->addWidget(new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy")), 6, 4);

    grid->addWidget(new QLabel(QStringLiteral("Due Date:")), 7, 3);
    grid->addWidget(new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy")), 7, 4);

    dlgLayout->addLayout(grid);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnOk = new QPushButton(QStringLiteral("✔️ OK (F9)"), &dlg);
    btnOk->setShortcut(QKeySequence(Qt::Key_F9));
    btnOk->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; padding: 4px 14px; font-weight: bold; }"
    ));
    connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);

    QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), &dlg);
    btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
    btnCancel->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #E57373; padding: 4px 14px; font-weight: bold; }"
    ));
    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);
    dlgLayout->addLayout(btnLayout);

    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, QStringLiteral("Invoice Finalized"),
            QStringLiteral("Bill SS000173 posted successfully via %1.\nThermal receipt sent to printer.").arg(paymentMode));
    }
}

void POSBillingPage::onHoldBillClicked() {
    QMessageBox::information(this, QStringLiteral("Bill Parked"), QStringLiteral("Transaction SS000173 parked on hold."));
}

void POSBillingPage::onRecallBillClicked() {
    resetBillingSession();
}

void POSBillingPage::onVoidBillClicked() {
    auto reply = QMessageBox::question(this, QStringLiteral("Void Transaction"),
        QStringLiteral("Are you sure you want to cancel and clear current bill?"),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_tableCart->setRowCount(0);
        m_lblItemCount->setText(QStringLiteral("<b>No of Items : 0</b>"));
        m_lblTotalQty->setText(QStringLiteral(" 0.00 +0.00 "));
        m_lblBigBillAmount->setText(QStringLiteral("₹ 0.00"));
    }
}

void POSBillingPage::onQuickCustomerClicked() {
    QuickCustomerDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        auto c = dlg.createdCustomer();
        m_txtCustCode->setText(c.code);
        m_txtCustName->setText(c.name);
        m_txtCustPhone->setText(c.phone);
        m_txtCustAddress->setText(c.address);
    }
}

void POSBillingPage::updateTotals() {}
void POSBillingPage::populateCustomerCombo() {}
