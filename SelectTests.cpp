#include "SelectTests.h"
#include "ui_SelectTests.h"

#include <QGraphicsDropShadowEffect>

QList<QCheckBox*> SelectTests::allCheckBoxes() const {
    return {
        ui->check_box_pressure_1, ui->check_box_pressure_2, ui->check_box_pressure_3,
        ui->check_box_moving, ui->check_box_input_4_20_mA, ui->check_box_output_4_20_mA, ui->check_box_usb,
        ui->check_box_imit_switch_0_3, ui->check_box_imit_switch_3_0,
        ui->check_box_do_1, ui->check_box_do_2, ui->check_box_do_3, ui->check_box_do_4
    };
}

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
            // QFont font = cb->font();
            // font.setWeight(checked ? QFont::DemiBold : QFont::Normal);
            // cb->setFont(font);

            frame->setStyleSheet(checked
                                 ? "background-color: #E1E1E1; border-radius: 12px;"
                                 : "" );
        });

        cb->setChecked(cb->isChecked());
    }

    for (QCheckBox* cb : allCheckBoxes())
        connect(cb, &QCheckBox::toggled, this, &SelectTests::onCheckBoxChanged);

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

void SelectTests::resetCheckBoxes() {
    for (QCheckBox* cb : allCheckBoxes()) {
        QSignalBlocker blocker(cb);
        cb->setChecked(false);
    }
}

void SelectTests::setPattern(const PatternSetup& setup) {
    for (QCheckBox* cb : allCheckBoxes()) {
        QSignalBlocker blocker(cb);
        cb->setChecked(false);
    }
    for (QCheckBox* cb : setup.checksOn) {
        QSignalBlocker blocker(cb);
        cb->setChecked(true);
    }
    for (QCheckBox* cb : setup.checksOff) {
        QSignalBlocker blocker(cb);
        cb->setChecked(false);
    }

    for (QCheckBox* cb : allCheckBoxes()) {
        emit cb->toggled(cb->isChecked());
    }

    onCheckBoxChanged();
    qDebug() << "[setPattern] После выставления паттерна: m_currentPattern =" << m_currentPattern;
}

bool SelectTests::isValidPattern() {
    // Комплексных; Отсечной Арматуры; Тесты: полного хода, циклический
    if (m_blockCts.usb && m_blockCts.imit_switch_0_3 && m_blockCts.imit_switch_3_0 &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4)) return true;
    // Базовых; Запорно-Регулирующей Арматуры; Тесты: полного хода, циклический
    if (m_blockCts.usb && m_blockCts.input_4_20_mA && m_blockCts.output_4_20_mA &&
        m_blockCts.imit_switch_0_3 && m_blockCts.imit_switch_3_0 &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4)) return true;
    // Комплексных; Запорно-Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
    if (m_blockCts.usb && m_blockCts.pressure_1 && m_blockCts.pressure_2 && m_blockCts.pressure_3 &&
        m_blockCts.moving && m_blockCts.input_4_20_mA && m_blockCts.output_4_20_mA &&
        m_blockCts.imit_switch_0_3 && m_blockCts.imit_switch_3_0 &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4)) return true;
    // Базовых; Регулирующей Арматуры; Тесты: полного хода, циклический
    if (m_blockCts.usb && m_blockCts.input_4_20_mA && m_blockCts.output_4_20_mA) return true;
    // Комплексных; Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
    if (m_blockCts.usb && m_blockCts.pressure_1 && m_blockCts.pressure_2 && m_blockCts.pressure_3 &&
        m_blockCts.moving && m_blockCts.input_4_20_mA && m_blockCts.output_4_20_mA) return true;
    return false;
}

SelectTests::PatternType SelectTests::detectCurrentPattern() const
{
    // Комплексных; Запорно-Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
    if (ui->check_box_usb->isChecked() &&
        ui->check_box_pressure_1->isChecked() &&
        ui->check_box_pressure_2->isChecked() &&
        ui->check_box_pressure_3->isChecked() &&
        ui->check_box_moving->isChecked() &&
        ui->check_box_input_4_20_mA->isChecked() &&
        ui->check_box_output_4_20_mA->isChecked() &&
        ui->check_box_imit_switch_0_3->isChecked() &&
        ui->check_box_imit_switch_3_0->isChecked() &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4))
    {
        return Pattern_CTSV;
    }

    // Комплексных; Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический
    if (ui->check_box_usb->isChecked() &&
        ui->check_box_pressure_1->isChecked() &&
        ui->check_box_pressure_2->isChecked() &&
        ui->check_box_pressure_3->isChecked() &&
        ui->check_box_moving->isChecked() &&
        ui->check_box_input_4_20_mA->isChecked() &&
        ui->check_box_output_4_20_mA->isChecked())
        // &&
        // !ui->check_box_imit_switch_0_3->isChecked() &&
        // !ui->check_box_imit_switch_3_0->isChecked() &&
        // !ui->check_box_do_1->isChecked() &&
        // !ui->check_box_do_2->isChecked() &&
        // !ui->check_box_do_3->isChecked() &&
        // !ui->check_box_do_4->isChecked())
    {
        return Pattern_CTCV;
    }

    // Базовых; Запорно-Регулирующей Арматуры; Тесты: полного хода, циклический
    if (ui->check_box_usb->isChecked() &&
        ui->check_box_input_4_20_mA->isChecked() &&
        ui->check_box_output_4_20_mA->isChecked() &&
        ui->check_box_imit_switch_0_3->isChecked() &&
        ui->check_box_imit_switch_3_0->isChecked() &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4))
        // &&
        // !ui->check_box_pressure_1->isChecked() &&
        // !ui->check_box_pressure_2->isChecked() &&
        // !ui->check_box_pressure_3->isChecked() &&
        // !ui->check_box_moving->isChecked() &&
        // !ui->check_box_do_2->isChecked() &&
        // !ui->check_box_do_3->isChecked() &&
        // !ui->check_box_do_4->isChecked())
    {
        return Pattern_BTSV;
    }

    // Комплексных; Отсечной Арматуры; Тесты: полного хода, циклический
    if (ui->check_box_usb->isChecked() &&
        ui->check_box_moving->isChecked() &&
        ui->check_box_imit_switch_3_0->isChecked() &&
        ui->check_box_imit_switch_0_3->isChecked() &&
        (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4))
        // &&
        // !ui->check_box_pressure_1->isChecked() &&
        // !ui->check_box_pressure_2->isChecked() &&
        // !ui->check_box_pressure_3->isChecked() &&
        // !ui->check_box_input_4_20_mA->isChecked() &&
        // !ui->check_box_output_4_20_mA->isChecked())
    {
        return Pattern_CTV;
    }

    // Базовых; Регулирующей Арматуры; Тесты: полного хода, циклический
    if (ui->check_box_usb->isChecked() &&
        ui->check_box_input_4_20_mA->isChecked() &&
        ui->check_box_output_4_20_mA->isChecked())
        // &&
        // !ui->check_box_pressure_1->isChecked() &&
        // !ui->check_box_pressure_2->isChecked() &&
        // !ui->check_box_pressure_3->isChecked() &&
        // !ui->check_box_moving->isChecked() &&
        // !ui->check_box_imit_switch_0_3->isChecked() &&
        // !ui->check_box_imit_switch_3_0->isChecked() &&
        // (m_blockCts.do_1 || m_blockCts.do_2 || m_blockCts.do_3 || m_blockCts.do_4))
    {
        return Pattern_BTCV;
    }

    return Pattern_None;
}

void SelectTests::onCheckBoxChanged()
{
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

    m_currentPattern = detectCurrentPattern();

    if (isValidPattern()) {
        ui->entry_testing->setEnabled(true);
    } else {
        ui->entry_testing->setEnabled(false);
    }

    if (!m_suppressDebugOutput) {
        switch (m_currentPattern) {
        case Pattern_CTV:
            qDebug() << "Текущий паттерн: Комплексных; Отсечной Арматуры; Тесты: полного хода, циклический";
            break;
        case Pattern_BTSV:
            qDebug() << "Текущий паттерн: Базовых; Запорно-Регулирующей Арматуры; Тесты: полного хода, циклический";
            break;
        case Pattern_CTSV:
            qDebug() << "Текущий паттерн: Комплексных; Запорно-Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический";
            break;
        case Pattern_BTCV:
            qDebug() << "Текущий паттерн: Базовых; Регулирующей Арматуры; Тесты: полного хода, циклический";
            break;
        case Pattern_CTCV:
            qDebug() << "Текущий паттерн: Комплексных; Регулирующей Арматуры; Тесты: основной, полного хода, опциональный, циклический";
            break;
        default:
            qDebug() << "Текущий паттерн: (кастомная/неизвестная комбинация)";
            break;
        }
    }
}

void SelectTests::ButtonClick_CTV() {
    setPattern({
        {ui->check_box_usb, ui->check_box_moving, ui->check_box_imit_switch_3_0, ui->check_box_imit_switch_0_3, ui->check_box_do_1},
        {}
    });
}

void SelectTests::ButtonClick_BTSV() {
    setPattern({
        {ui->check_box_usb, ui->check_box_output_4_20_mA, ui->check_box_input_4_20_mA,
         ui->check_box_imit_switch_0_3, ui->check_box_imit_switch_3_0, ui->check_box_do_1},
        {}
    });
}

void SelectTests::ButtonClick_CTSV() {
    setPattern({
        {ui->check_box_usb, ui->check_box_pressure_1, ui->check_box_pressure_2, ui->check_box_pressure_3,
         ui->check_box_moving, ui->check_box_output_4_20_mA, ui->check_box_input_4_20_mA,
         ui->check_box_imit_switch_0_3, ui->check_box_imit_switch_3_0, ui->check_box_do_1},
        {}
    });
}

void SelectTests::ButtonClick_BTCV() {
    setPattern({
        {ui->check_box_usb, ui->check_box_output_4_20_mA, ui->check_box_input_4_20_mA, ui->check_box_do_1},
        {}
    });
}

void SelectTests::ButtonClick_CTCV() {
    setPattern({
        {ui->check_box_usb, ui->check_box_pressure_1, ui->check_box_pressure_2, ui->check_box_pressure_3,
         ui->check_box_moving, ui->check_box_output_4_20_mA, ui->check_box_input_4_20_mA},
        {}
    });
}

SelectTests::PatternType SelectTests::currentPattern() const {
    return m_currentPattern;
}
void SelectTests::ButtonClick() {
    accept();
}



