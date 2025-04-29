#include "SelectTests.h"
#include "ui_SelectTests.h"

SelectTests::SelectTests(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectTests)
{
    ui->setupUi(this);

    // Цвет виджетов (если это необходимо)
    ui->pressure_1_color->setStyleSheet("background-color: #2A689F;");
    ui->pressure_2_color->setStyleSheet("background-color: #457448;");
    ui->pressure_3_color->setStyleSheet("background-color: #D3BB2A;");
    ui->moving_color->setStyleSheet("background-color: #CC9546;");
    ui->input_4_20_mA_color->setStyleSheet("background-color: #A83240;");
    ui->output_4_20_mA_color->setStyleSheet("background-color: #A83240;");
    ui->usb_color->setStyleSheet("background-color: #25262B;");
    ui->imit_switch_0_3_color->setStyleSheet("background-color: #463564;");
    ui->imit_switch_3_0_color->setStyleSheet("background-color: #463564;");
    ui->do_1_color->setStyleSheet("background-color: #25262B;");
    ui->do_2_color->setStyleSheet("background-color: #25262B;");
    ui->do_3_color->setStyleSheet("background-color: #25262B;");
    ui->do_4_color->setStyleSheet("background-color: #25262B;");

    ui->entry_testing->setEnabled(false);

    // Подключаем сигналы для чекбоксов
    connect(ui->check_box_pressure_1, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_pressure_2, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_pressure_3, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_moving, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_input_4_20_mA, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_output_4_20_mA, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_usb, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_imit_switch_0_3, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_imit_switch_3_0, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_do_1, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_do_2, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_do_3, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);
    connect(ui->check_box_do_4, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);

    connect(ui->button_CTV, &QPushButton::clicked, this, &SelectTests::ButtonClick_CTV);
    connect(ui->button_BTSV, &QPushButton::clicked, this, &SelectTests::ButtonClick_BTSV);
    connect(ui->button_CTSV, &QPushButton::clicked, this, &SelectTests::ButtonClick_CTSV);
    connect(ui->button_BTCV, &QPushButton::clicked, this, &SelectTests::ButtonClick_BTCV);
    connect(ui->button_CTCV, &QPushButton::clicked, this, &SelectTests::ButtonClick_CTCV);

    connect(ui->entry_testing, &QPushButton::clicked, this, &SelectTests::ButtonClick);
}

SelectTests::~SelectTests()
{
    delete ui;
}

SelectTests::BlockCTS SelectTests::getCTS() const
{
    return m_blockCts;
}

bool SelectTests::isValidPattern(const BlockCTS& block_cts) {
    // Комплексных; Отсечной Арматуры; Тесты: полного хода, циклический
    if (m_blockCts.usb &&
        m_blockCts.imit_switch_0_3 &&
        m_blockCts.imit_switch_3_0 &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4)
        )
    {
        return true;
    }
    // Базовых; Запорно-Регулирующей Арматуры; Тесты: полного хода, циклический
    else if (m_blockCts.usb &&
             m_blockCts.input_4_20_mA &&
             m_blockCts.output_4_20_mA &&
             m_blockCts.imit_switch_0_3 &&
             m_blockCts.imit_switch_3_0 &&
             (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4)
             )
    {
        return true;
    }
    // Комплексных; Запорно-Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
    else if (m_blockCts.usb &&
             m_blockCts.pressure_1 &&
             m_blockCts.pressure_2 &&
             m_blockCts.pressure_3 &&
             m_blockCts.moving &&
             m_blockCts.input_4_20_mA &&
             m_blockCts.output_4_20_mA &&
             m_blockCts.imit_switch_0_3 &&
             m_blockCts.imit_switch_3_0 &&
             (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4)
             )
    {
        return true;
    }
    // Базовых; Регулирующей Арматуры; Тесты: полного ходад, циклический
    else if (m_blockCts.usb &&
             m_blockCts.input_4_20_mA &&
             m_blockCts.output_4_20_mA
             )
    {
        return true;
    }
    // Комплексных; Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
    else if (m_blockCts.usb &&
             m_blockCts.pressure_1 &&
             m_blockCts.pressure_2 &&
             m_blockCts.pressure_3 &&
             m_blockCts.moving &&
             m_blockCts.input_4_20_mA &&
             m_blockCts.output_4_20_mA
             )
    {
        return true;
    }
    return false;
}


void SelectTests::onCheckBoxChanged()
{
    // Обновляем значения в block_cts на основе текущих состояний чекбоксов
    m_blockCts.pressure_1 = ui->check_box_pressure_1->isChecked();
    m_blockCts.pressure_2 = ui->check_box_pressure_2->isChecked();
    m_blockCts.pressure_3 = ui->check_box_pressure_3->isChecked();
    m_blockCts.moving = ui->check_box_moving->isChecked();
    m_blockCts.input_4_20_mA = ui->check_box_input_4_20_mA->isChecked();
    m_blockCts.output_4_20_mA = ui->check_box_output_4_20_mA->isChecked();
    m_blockCts.usb = ui->check_box_usb->isChecked();
    m_blockCts.imit_switch_0_3 = ui->check_box_imit_switch_0_3->isChecked();
    m_blockCts.imit_switch_3_0 = ui->check_box_imit_switch_3_0->isChecked();
    m_blockCts.do_1 = ui->check_box_do_1->isChecked();
    m_blockCts.do_2 = ui->check_box_do_2->isChecked();
    m_blockCts.do_3 = ui->check_box_do_3->isChecked();
    m_blockCts.do_4 = ui->check_box_do_4->isChecked();


    if (isValidPattern(m_blockCts)) {
        ui->entry_testing->setEnabled(true);  // Включаем кнопку, если паттерн выбран
    } else {
        ui->entry_testing->setEnabled(false); // Выключаем кнопку, если паттерн не выбран
    }
}

void SelectTests::resetCheckBoxes() {
    // Отключаем все чекбоксы
    ui->check_box_pressure_1->setChecked(false);
    ui->check_box_pressure_2->setChecked(false);
    ui->check_box_pressure_3->setChecked(false);
    ui->check_box_moving->setChecked(false);
    ui->check_box_input_4_20_mA->setChecked(false);
    ui->check_box_output_4_20_mA->setChecked(false);
    ui->check_box_usb->setChecked(false);
    ui->check_box_imit_switch_0_3->setChecked(false);
    ui->check_box_imit_switch_3_0->setChecked(false);
    ui->check_box_do_1->setChecked(false);
    ui->check_box_do_2->setChecked(false);
    ui->check_box_do_3->setChecked(false);
    ui->check_box_do_4->setChecked(false);
}

void SelectTests::ButtonClick_CTV() {
    resetCheckBoxes();
    ui->check_box_usb->setChecked(true);
    ui->check_box_moving->setChecked(true);
    ui->check_box_imit_switch_3_0->setChecked(true);
    ui->check_box_imit_switch_0_3->setChecked(true);
    ui->check_box_do_1->setChecked(true);
    onCheckBoxChanged();
}

void SelectTests::ButtonClick_BTSV() {
    resetCheckBoxes();
    ui->check_box_usb->setChecked(true);
    ui->check_box_output_4_20_mA->setChecked(true);
    ui->check_box_input_4_20_mA->setChecked(true);
    ui->check_box_imit_switch_0_3->setChecked(true);
    ui->check_box_imit_switch_3_0->setChecked(true);
    ui->check_box_do_1->setChecked(true);
    onCheckBoxChanged();
}

void SelectTests::ButtonClick_CTSV() {
    resetCheckBoxes();
    ui->check_box_usb->setChecked(true);
    ui->check_box_pressure_1->setChecked(true);
    ui->check_box_pressure_2->setChecked(true);
    ui->check_box_pressure_3->setChecked(true);
    ui->check_box_moving->setChecked(true);
    ui->check_box_output_4_20_mA->setChecked(true);
    ui->check_box_input_4_20_mA->setChecked(true);
    ui->check_box_imit_switch_0_3->setChecked(true);
    ui->check_box_imit_switch_3_0->setChecked(true);
    ui->check_box_do_1->setChecked(true);
    onCheckBoxChanged();
}

void SelectTests::ButtonClick_BTCV() {
    resetCheckBoxes();
    ui->check_box_usb->setChecked(true);
    ui->check_box_output_4_20_mA->setChecked(true);
    ui->check_box_input_4_20_mA->setChecked(true);
    onCheckBoxChanged();
}

void SelectTests::ButtonClick_CTCV() {
    resetCheckBoxes();
    ui->check_box_usb->setChecked(true);
    ui->check_box_pressure_1->setChecked(true);
    ui->check_box_pressure_2->setChecked(true);
    ui->check_box_pressure_3->setChecked(true);
    ui->check_box_moving->setChecked(true);
    ui->check_box_output_4_20_mA->setChecked(true);
    ui->check_box_input_4_20_mA->setChecked(true);
    onCheckBoxChanged();
}


void SelectTests::ButtonClick() {
    accept();
}



