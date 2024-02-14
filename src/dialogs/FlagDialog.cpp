#include "FlagDialog.h"
#include "ui_FlagDialog.h"

#include <QIntValidator>
#include <iostream>
#include "core/Cutter.h"

FlagDialog::FlagDialog(RVA offset, QWidget *parent)
    : QDialog(parent), ui(new Ui::FlagDialog), offset(offset), flagName(""), flagOffset(RVA_INVALID)
{
    std::cout << "offset" << " " << offset << std::endl;
    std::cout << "parent" << " " << parent << std::endl;
    std::cout << "parentObjectName" << " " << parent->objectName().toStdString() << std::endl;
    // Setup UI
    ui->setupUi(this);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
    RzFlagItem *flag = rz_flag_get_i(Core()->core()->flags, offset);
    std::cout << "flag" << " " << flag << std::endl;
    if (flag) {
        flagName = QString(flag->name);
        flagOffset = flag->offset;
    }

    auto size_validator = new QIntValidator(ui->sizeEdit);
    size_validator->setBottom(1);
    ui->sizeEdit->setValidator(size_validator);
    if (flag) {
        ui->nameEdit->setText(flag->name);
        ui->labelAction->setText(tr("Edit flag at %1").arg(RzAddressString(offset)));
    } else {
        std::cout << "offset in if condition" << " " << offset << std::endl;
        ui->labelAction->setText(tr("Add flag at %1").arg(RzAddressString(offset)));
    }

    // Connect slots
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FlagDialog::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FlagDialog::buttonBoxRejected);
}

FlagDialog::~FlagDialog() {}

void FlagDialog::buttonBoxAccepted()
{
    RVA size = ui->sizeEdit->text().toULongLong();
    QString name = ui->nameEdit->text();
    std::cout << "name" << " " << name.toStdString() << std::endl;

    if (name.isEmpty()) {
        if (flagOffset != RVA_INVALID) {
            // Empty name and flag exists -> delete the flag
            Core()->delFlag(flagOffset);
        } else {
            // Flag was not existing and we gave an empty name, do nothing
        }
    } else {
        if (flagOffset != RVA_INVALID) {
            // Name provided and flag exists -> rename the flag
            Core()->renameFlag(flagName, name);
        } else {
            // Name provided and flag does not exist -> create the flag
            Core()->addFlag(offset, name, size);
            std::cout << "Adding flag" << std::endl;
        }
    }
    close();
    this->setResult(QDialog::Accepted);
}

void FlagDialog::buttonBoxRejected()
{
    close();
    this->setResult(QDialog::Rejected);
}
