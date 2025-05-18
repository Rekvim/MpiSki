#include "SelectTests.h"
#include "ui_SelectTests.h"

#include <QGraphicsDropShadowEffect>

SelectTests::SelectTests(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectTests)
{
    ui->setupUi(this);



    ui->entry_testing->setEnabled(false);

    QMap<QCheckBox*, QFrame*> checkToFrame = {
        { ui->check_box_pressure_1, ui->frame_pressure_1 },
        { ui->check_box_pressure_2, ui->frame_pressure_2 },
        { ui->check_box_pressure_3, ui->frame_pressure_3 },
        { ui->check_box_moving, ui->frame_moving },
        { ui->check_box_input_4_20_mA, ui->frame_input_4_20_mA },
        { ui->check_box_output_4_20_mA, ui->frame_output_4_20_mA },
        { ui->check_box_usb, ui->frame_usb },
        { ui->check_box_imit_switch_0_3, ui->frame_imit_switch_0_3 },
        { ui->check_box_imit_switch_3_0, ui->frame_imit_switch_3_0 },
        { ui->check_box_do_1, ui->frame_do_1 },
        { ui->check_box_do_2, ui->frame_do_2 },
        { ui->check_box_do_3, ui->frame_do_3 },
        { ui->check_box_do_4, ui->frame_do_4 }
    };

    for (auto it = checkToFrame.begin(); it != checkToFrame.end(); ++it) {
        QCheckBox* cb = it.key();
        QFrame* frame = it.value();

        connect(cb, &QCheckBox::toggled, this, [cb, frame](bool checked) {
            QFont font = cb->font();
            font.setWeight(checked ? QFont::DemiBold : QFont::Normal);
            cb->setFont(font);

            if (checked) {
                frame->setStyleSheet("background-color: #E1E1E1; border-radius: 6px;");
            } else {
                frame->setStyleSheet("");
            }
        });

        cb->setChecked(cb->isChecked());
    }


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

bool SelectTests::isValidPattern() {
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


    if (isValidPattern()) {
        ui->entry_testing->setEnabled(true);
    } else {
        ui->entry_testing->setEnabled(false);
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



