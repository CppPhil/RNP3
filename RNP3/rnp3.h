#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_rnp3.h"
#include "client.hpp"

namespace gui {
    class RNP3 : public QMainWindow {
        Q_OBJECT

    public:
        explicit RNP3(qint16 port, QWidget *parent = nullptr);

    private slots:
        void drawResponse(QString dataToDisplay) const;

    private:
        Ui::RNP3Class ui;
        app::Client client_;
    }; // END of class RNP3
} // END of namespace gui
