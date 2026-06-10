#pragma once

#include <QDialog>

class ScanController;
class QDoubleSpinBox;

class ManualAlignDialog : public QDialog {
    Q_OBJECT
public:
    explicit ManualAlignDialog(ScanController* ctrl, QWidget* parent = nullptr);
    ~ManualAlignDialog();

private slots:
    void onValuesChanged();
    void onResetClicked();
    void onAccepted();
    void onRejected();

private:
    ScanController* m_ctrl;

    QDoubleSpinBox* m_spnTx;
    QDoubleSpinBox* m_spnTy;
    QDoubleSpinBox* m_spnTz;

    QDoubleSpinBox* m_spnRx;
    QDoubleSpinBox* m_spnRy;
    QDoubleSpinBox* m_spnRz;
};
