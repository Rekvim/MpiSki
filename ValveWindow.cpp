#include <QMessageBox>
#include "ValveWindow.h"
#include "ui_ValveWindow.h"

ValveWindow::ValveWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ValveWindow)
{
    ui->setupUi(this);

    QRegularExpression regular;
    regular.setPattern("[@!^/?*:;{}\\\\]+");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regular, this);

    ui->lineEdit_position->setValidator(validator);
    ui->lineEdit_manufacturer->setValidator(validator);
    ui->lineEdit_valve_serie->setValidator(validator);
    // ui->lineEdit_valve_model->setValidator(validator);
    ui->lineEdit_serial->setValidator(validator);
    ui->lineEdit_DN->setValidator(validator);
    ui->lineEdit_PN->setValidator(validator);
    ui->lineEdit_stroke->setValidator(validator);
    ui->lineEdit_positioner->setValidator(validator);
    ui->lineEdit_dinamic_error->setValidator(validator);
    ui->lineEdit_model_drive->setValidator(validator);
    ui->lineEdit_range->setValidator(validator);
    ui->lineEdit_material->setValidator(validator);
    ui->lineEdit_material_corpus->setValidator(validator);
    ui->lineEdit_material_cap->setValidator(validator);
    ui->lineEdit_material_ball->setValidator(validator);
    ui->lineEdit_material_disk->setValidator(validator);
    ui->lineEdit_material_plunger->setValidator(validator);
    ui->lineEdit_material_shaft->setValidator(validator);
    ui->lineEdit_material_stock->setValidator(validator);
    ui->lineEdit_material_guide_sleeve->setValidator(validator);



    ui->doubleSpinBox_diameter_pulley->setValue(diameter_[0]);

    connect(ui->comboBox_stroke_movement,
            &QComboBox::currentIndexChanged,
            this,
            &ValveWindow::StrokeChanged);
    connect(ui->comboBox_tool_number,
            &QComboBox::currentIndexChanged,
            this,
            &ValveWindow::ToolChanged);
    connect(ui->doubleSpinBox_diameter,
            &QDoubleSpinBox::valueChanged,
            this,
            &ValveWindow::DiameterChanged);

    connect(ui->pushButton, &QPushButton::clicked, this, &ValveWindow::ButtonClick);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &ValveWindow::Clear);

    DiameterChanged(ui->doubleSpinBox_diameter->value());
    loadDNValues();
    connect(ui->comboBox_DN, &QComboBox::currentTextChanged,
            this, &ValveWindow::on_comboBox_DN_currentIndexChanged);
}

void ValveWindow::loadDNValues()
{
    QFile file(":/db_valve_data/valve_data.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть valve_data.json";
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Ошибка парсинга JSON";
        return;
    }

    QJsonObject rootObj = doc.object();

    // Добавление произодителей
    QJsonArray manufacturerArray = rootObj.value("manufacturer").toArray();
    ui->comboBox_manufacturer->clear();
    for (const QJsonValue &value : manufacturerArray) {
        ui->comboBox_manufacturer->addItem(value.toString());
    }

    // Добавление серии клапана
    QJsonArray valveSerialArray = rootObj.value("valveSeries").toArray();
    ui->comboBox_valve_serie->clear();
    for (const QJsonValue &value : valveSerialArray) {
        ui->comboBox_valve_serie->addItem(value.toString());
    }

    // Добавление модель клапана
    QJsonArray valveModelArray = rootObj.value("valveModel").toArray();
    ui->comboBox_valve_model->clear();
    for (const QJsonValue &value : valveModelArray) {
        ui->comboBox_valve_model->addItem(value.toString());
    }

    // Добавление модель привода
    QJsonArray driveModelArray = rootObj.value("driveModel").toArray();
    ui->comboBox_drive_model->clear();
    for (const QJsonValue &value : driveModelArray) {
        ui->comboBox_drive_model->addItem(value.toString());
    }

    // Добавление материалов седла
    QJsonArray saddleMaterialsArray = rootObj.value("saddleMaterials").toArray();
    ui->comboBox_saddle_materials->clear();
    for (const QJsonValue &value : saddleMaterialsArray) {
        ui->comboBox_saddle_materials->addItem(value.toString());
    }

    // Добавление материалов корпуса
    QJsonArray bodyMaterialsArray = rootObj.value("bodyMaterials").toArray();
    ui->comboBox_material_body->clear();
    for (const QJsonValue &value : bodyMaterialsArray) {
        ui->comboBox_material_body->addItem(value.toString());
    }

    // Параметры клапана
    QJsonObject valveDataObj = rootObj.value("DN").toObject();
    // Получаем список ключей DN и сортируем их по числовому значению
    QStringList dnKeys = valveDataObj.keys();
    std::sort(dnKeys.begin(), dnKeys.end(), [](const QString &a, const QString &b) {
        return a.toInt() < b.toInt();
    });

    ui->comboBox_DN->clear();
    ui->comboBox_DN->addItems(dnKeys);
    // ui->comboBox_DN->addItem(manual_input_);

    // Сохраняем объект valve_data в члене класса для дальнейшего использования
    m_valveDataObj = valveDataObj;
    // Гарант, что для первого элемента valveDataObj сразу заполнится comboBox_CV.
    if (ui->comboBox_DN->count() > 0) {
        ui->comboBox_DN->setCurrentIndex(0);
        on_comboBox_DN_currentIndexChanged(ui->comboBox_DN->currentText());
    }
}

void ValveWindow::on_comboBox_DN_currentIndexChanged(const QString &selectedDN)
{
    if (selectedDN.isEmpty())
        return;

    // Из ранее сохранённого m_valveDataObj получаем массив вариантов для выбранного DN
    QJsonArray variantsArray = m_valveDataObj.value(selectedDN).toArray();

    ui->comboBox_CV->clear();
    // Вывод CV из DN
    for (const QJsonValue &val : variantsArray) {
        QJsonObject variant = val.toObject();
        double cvValue = variant.value("CV").toDouble();
        ui->comboBox_CV->addItem(QString::number(cvValue));
    }

}

ValveWindow::~ValveWindow()
{
    delete ui;
}

void ValveWindow::SetRegistry(Registry *registry)
{
    registry_ = registry;

    ui->comboBox_position->clear();
    ui->comboBox_position->addItems(registry_->GetPositions());
    ui->comboBox_position->addItem(manual_input_);

    QString last_position = registry_->GetLastPosition();
    if (last_position == "") {
        ui->comboBox_position->setCurrentIndex(ui->comboBox_position->count() - 1);
    } else {
        ui->comboBox_position->setCurrentIndex(ui->comboBox_position->findText(last_position));
        PositionChanged(last_position);
    }

    connect(ui->comboBox_position,
            &QComboBox::currentTextChanged,
            this,
            &ValveWindow::PositionChanged);
}

void ValveWindow::SaveValveInfo()
{
    if (ui->comboBox_position->currentText() == "Ручной ввод")
        valve_info_ = registry_->GetValveInfo(ui->lineEdit_position->text());

    if (ui->comboBox_position->currentText() == "Ручной ввод")
        valve_info_->manufacturer = ui->lineEdit_manufacturer->text();


    // valve_info_->model = ui->lineEdit_valve_model->text();
    valve_info_->serial = ui->lineEdit_serial->text();
    valve_info_->DN = ui->lineEdit_DN->text();
    valve_info_->PN = ui->lineEdit_PN->text();
    valve_info_->stroke = ui->lineEdit_stroke->text();
    valve_info_->positioner = ui->lineEdit_positioner->text();
    valve_info_->dinamic_error = ui->lineEdit_dinamic_error->text();
    valve_info_->model_drive = ui->lineEdit_model_drive->text();
    valve_info_->range = ui->lineEdit_range->text();
    valve_info_->material = ui->lineEdit_material->text();
    valve_info_->manufacturer = ui->lineEdit_manufacturer->text();
    valve_info_->manufacturer = ui->lineEdit_manufacturer->text();

    valve_info_->diameter = ui->doubleSpinBox_diameter->value();
    valve_info_->pulley = ui->doubleSpinBox_diameter_pulley->value();

    valve_info_->normal_position = ui->comboBox_normal_position->currentIndex();
    valve_info_->drive_type = ui->comboBox_type_of_drive->currentIndex();
    valve_info_->stroke_movement = ui->comboBox_stroke_movement->currentIndex();
    valve_info_->tool_number = ui->comboBox_tool_number->currentIndex();

    valve_info_->corpus = ui->lineEdit_material_corpus->text();
    valve_info_->cap = ui->lineEdit_material_cap->text();

    QString selectedCV = ui->comboBox_CV->currentText();
    valve_info_->saddle_cv = selectedCV;

    QString selectedMaterialSaddle = ui->comboBox_saddle_materials->currentText();
    valve_info_->saddle_materials = selectedMaterialSaddle;
    valve_info_->ball = ui->lineEdit_material_ball->text();
    valve_info_->disk = ui->lineEdit_material_disk->text();
    valve_info_->plunger = ui->lineEdit_material_plunger->text();
    valve_info_->shaft = ui->lineEdit_material_shaft->text();
    valve_info_->stock = ui->lineEdit_material_stock->text();
    valve_info_->guide_sleeve = ui->lineEdit_material_guide_sleeve->text();

    registry_->SaveValveInfo();
}

void ValveWindow::PositionChanged(const QString &position)
{
    if (position == "Ручной ввод") {
        ui->lineEdit_position->setEnabled(true);
        return;
    }

    // ui->lineEdit_manufacturer->setEnabled(false);
    // connect(ui->comboBox_manufacturer, &QComboBox::currentTextChanged,
    //         this, [this](const QString &text) {
    //         ui->lineEdit_valve_model->setEnabled(text == manual_input_);
    // });

    ui->lineEdit_valve_serie->setEnabled(false);
    connect(ui->comboBox_valve_model, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                ui->lineEdit_valve_serie->setEnabled(text == manual_input_);
            });

    ui->lineEdit_valve_serie->setEnabled(false);
    connect(ui->comboBox_valve_serie, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
            ui->lineEdit_valve_serie->setEnabled(text == manual_input_);
    });

    ui->lineEdit_DN->setEnabled(false);
    connect(ui->comboBox_DN, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                ui->lineEdit_DN->setEnabled(text == manual_input_);
            });

    // connect(ui->comboBox_position, &QComboBox::currentTextChanged,
    //         this, [this](const QString &text) {
    //             ui->lineEdit_position->setEnabled(text == "Ручной ввод");
    //         });

    // connect(ui->comboBox_material_body, &QComboBox::currentTextChanged,
    //         this, [this](const QString &text) {
    //             ui->lineEdit_material_corpus->setEnabled(text == "Ручной ввод");
    //         });

    // connect(ui->comboBox_DN, &QComboBox::currentTextChanged,
    //         this, [this](const QString &text) {
    //             ui->lineEdit_DN->setEnabled(text == "Ручной ввод");
    //         });


    valve_info_ = registry_->GetValveInfo(position);

    ui->lineEdit_position->setText(position);
    ui->lineEdit_position->setEnabled(false);
    ui->lineEdit_material_corpus->setEnabled(false);
    // ui->lineEdit_material_cv->setEnabled(false);

    ui->lineEdit_manufacturer->setText(valve_info_->manufacturer);
    // ui->lineEdit_valve_model->setText(valve_info_->model);
    ui->lineEdit_serial->setText(valve_info_->serial);
    ui->lineEdit_DN->setText(valve_info_->DN);
    ui->lineEdit_PN->setText(valve_info_->PN);
    ui->lineEdit_stroke->setText(valve_info_->stroke);
    ui->lineEdit_positioner->setText(valve_info_->positioner);
    ui->lineEdit_dinamic_error->setText(valve_info_->dinamic_error);
    ui->lineEdit_model_drive->setText(valve_info_->model_drive);
    ui->lineEdit_range->setText(valve_info_->range);
    ui->lineEdit_material->setText(valve_info_->material);

    ui->doubleSpinBox_diameter->setValue(valve_info_->diameter);

    ui->comboBox_normal_position->setCurrentIndex(valve_info_->normal_position);
    ui->comboBox_type_of_drive->setCurrentIndex(valve_info_->drive_type);
    ui->comboBox_stroke_movement->setCurrentIndex(valve_info_->stroke_movement);
    ui->comboBox_tool_number->setCurrentIndex(valve_info_->tool_number);
    ui->doubleSpinBox_diameter_pulley->setValue(valve_info_->pulley);

    ui->lineEdit_material_corpus->setText(valve_info_->corpus);
    ui->lineEdit_material_cap->setText(valve_info_->cap);

    // QStringList parts = valve_info_->saddle_cv_kv.split(" ", Qt::SkipEmptyParts); // new
    // if (parts.size() >= 2) {
    //     ui->lineEdit_material_cv->setText(parts.at(0));
    //     ui->lineEdit_material_kv->setText(parts.at(1));
    // } else if (parts.size() == 1) {
    //     ui->lineEdit_material_cv->setText(parts.at(0));
    //     ui->lineEdit_material_kv->clear();
    // } else {
    //     ui->lineEdit_material_cv->clear();
    //     ui->lineEdit_material_kv->clear();
    // }

    // ui->lineEdit_material_cv->setText(valve_info_->saddle_cv_kv);
    ui->lineEdit_material_ball->setText(valve_info_->ball);
    ui->lineEdit_material_disk->setText(valve_info_->disk);
    ui->lineEdit_material_plunger->setText(valve_info_->plunger);
    ui->lineEdit_material_shaft->setText(valve_info_->shaft);
    ui->lineEdit_material_stock->setText(valve_info_->stock);
    ui->lineEdit_material_guide_sleeve->setText(valve_info_->guide_sleeve);
}

void ValveWindow::ButtonClick()
{
    if (ui->lineEdit_position->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите номер позиции");
        return;
    }

    // if ((ui->lineEdit_manufacturer->text().isEmpty()) or (ui->lineEdit_valve_model->text().isEmpty())
    //     or (ui->lineEdit_serial->text().isEmpty()) or (ui->lineEdit_DN->text().isEmpty())
    //     or (ui->lineEdit_PN->text().isEmpty()) or (ui->lineEdit_stroke->text().isEmpty())
    //     or (ui->lineEdit_positioner->text().isEmpty())
    //     or (ui->lineEdit_dinamic_error->text().isEmpty())
    //     or (ui->lineEdit_model_drive->text().isEmpty()) or (ui->lineEdit_range->text().isEmpty())
    //     or (ui->lineEdit_material->text().isEmpty())) {
    //     QMessageBox::StandardButton button
    //         = QMessageBox::question(this,
    //                                 "Предупреждение",
    //                                 "Введены не все данные, вы действительно хотети продолжить?");

    //     if (button == QMessageBox::StandardButton::No) {
    //         return;
    //     }
    // }

    OtherParameters *other_parameters = registry_->GetOtherParameters();
    other_parameters->normal_position = ui->comboBox_normal_position->currentText();
    other_parameters->movement = ui->comboBox_stroke_movement->currentText();
    SaveValveInfo();

    accept();
}

void ValveWindow::StrokeChanged(quint16 n)
{
    ui->comboBox_tool_number->setEnabled(n == 1);
    ui->doubleSpinBox_diameter_pulley->setEnabled(
        (n == 1)
        && (ui->comboBox_tool_number->currentIndex() == ui->comboBox_tool_number->count() - 1));
}

void ValveWindow::ToolChanged(quint16 n)
{
    if (n == ui->comboBox_tool_number->count() - 1) {
        ui->doubleSpinBox_diameter_pulley->setEnabled(true);
    } else {
        ui->doubleSpinBox_diameter_pulley->setEnabled(false);
        ui->doubleSpinBox_diameter_pulley->setValue(diameter_[n]);
    }
}

void ValveWindow::DiameterChanged(qreal value)
{
    ui->label_square->setText(QString().asprintf("%.2f", M_PI * value * value / 4));
}

void ValveWindow::Clear()
{
    ui->lineEdit_manufacturer->setText("");
    // ui->lineEdit_valve_model->setText("");
    ui->lineEdit_serial->setText("");
    ui->lineEdit_DN->setText("");
    ui->lineEdit_PN->setText("");
    ui->lineEdit_stroke->setText("");
    ui->lineEdit_positioner->setText("");
    ui->lineEdit_dinamic_error->setText("");
    ui->lineEdit_model_drive->setText("");
    ui->lineEdit_range->setText("");
    ui->lineEdit_material->setText("");

    ui->doubleSpinBox_diameter->setValue(1.0);

    ui->comboBox_normal_position->setCurrentIndex(0);
    ui->comboBox_type_of_drive->setCurrentIndex(0);
    ui->comboBox_stroke_movement->setCurrentIndex(0);
    ui->comboBox_tool_number->setCurrentIndex(0);
    ui->comboBox_CV->setCurrentIndex(0);
    ui->doubleSpinBox_diameter_pulley->setValue(diameter_[0]);

    ui->lineEdit_material_corpus->setText("");
    ui->lineEdit_material_cap->setText("");
    ui->lineEdit_material_ball->setText("");
    ui->lineEdit_material_disk->setText("");
    ui->lineEdit_material_plunger->setText("");
    ui->lineEdit_material_shaft->setText("");
    ui->lineEdit_material_stock->setText("");
    ui->lineEdit_material_guide_sleeve->setText("");
}
