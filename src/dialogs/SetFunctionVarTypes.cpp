#include "SetFunctionVarTypes.h"
#include "ui_SetFunctionVarTypes.h"

#include <QMetaType>
#include <QComboBox>

SetFunctionVarTypes::SetFunctionVarTypes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetFunctionVarTypes)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_OkPressed()));
    connect(ui->dropdownLocalVars, SIGNAL(currentIndexChanged(QString)), SLOT(on_ComboBoxChanged(QString)));

    populateTypesComboBox();

}

SetFunctionVarTypes::~SetFunctionVarTypes()
{
    delete ui;
}

void SetFunctionVarTypes::setUserMessage(const QString userMessage)
{
    ui->userMessage->setText(userMessage);
}

void SetFunctionVarTypes::setFcn(RCore* core, RAnalFunction *fcn)
{
    RListIter *iter;
    RAnalVar *var;

    if(!fcn)
    {
        ui->userMessage->setText(tr("You must be in a function to define variable types."));
        ui->labelSetTypeTo->setVisible(false);
        ui->selectedTypeForVar->setVisible(false);
        ui->dropdownLocalVars->setVisible(false);

        this->validFcn = false;
        return;
    }
    ui->userMessage->setVisible(true);
    ui->labelSetTypeTo->setVisible(true);
    ui->selectedTypeForVar->setVisible(true);
    ui->dropdownLocalVars->setVisible(true);

    this->validFcn = true;
    this->fcn = fcn;
    this->fcnVars = r_anal_var_list(core->anal, this->fcn, R_ANAL_VAR_KIND_ANY);

    for (iter = this->fcnVars->head; iter && (var = static_cast<RAnalVar*>(iter->data)); iter = iter->n)
    {
        ui->dropdownLocalVars->addItem(var->name,
                                       QVariant::fromValue(static_cast<void*>(var)));
    }
}

void SetFunctionVarTypes::on_OkPressed()
{

    RAnalVar *selectedVar;
    QVariant selectedVarVariant;

    if(!this->validFcn)
    {
        return;
    }

    selectedVarVariant = ui->dropdownLocalVars->currentData();
    selectedVar = static_cast<RAnalVar*>(selectedVarVariant.value<void*>());

    Core()->cmd(QString("afvt %1 %2")
                .arg(selectedVar->name)
                .arg(ui->selectedTypeForVar->currentText().toStdString().c_str()));

    //TODO: Switch argument names when new version of afvn new_type (old_type) is implmented
    Core()->cmd(QString("afvn %1 %2")
                .arg(ui->dropdownLocalVars->currentText().toStdString().c_str())
                .arg(ui->newVarName->text().replace(" ", "_").toStdString().c_str()));
}

void SetFunctionVarTypes::on_ComboBoxChanged(QString varName)
{
    ui->newVarName->setText(varName);
}


QStringList SetFunctionVarTypes::populateTypesComboBox()
{
    //gets all loaded types, structures and enums and puts them in a list

    QString res;
    QStringList userStructures;
    QStringList userEnumerations;
    QList<TypeDescription> primitiveTypesTypeList;

    res = Core()->cmd(QString("ts"));
    userStructures = res.split("\n");
    userStructures.removeAll(QString(""));
    ui->selectedTypeForVar->addItems(userStructures);
    ui->selectedTypeForVar->insertSeparator(ui->selectedTypeForVar->count());

    primitiveTypesTypeList = Core()->getAllTypes();

    for (TypeDescription thisType : primitiveTypesTypeList) {
        ui->selectedTypeForVar->addItem(thisType.type);
    }

    ui->selectedTypeForVar->insertSeparator(ui->selectedTypeForVar->count());

    res = Core()->cmd(QString("te"));
    userStructures = res.split("\n");
    userStructures.removeAll(QString(""));
    ui->selectedTypeForVar->addItems(userStructures);

    return

}
