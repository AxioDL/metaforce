#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.hpp"

extern const size_t MAINICON_QT_SZ;
extern const uint8_t MAINICON_QT[];

static QIcon MakeAppIcon()
{
    QIcon ret;

    const uint8_t* ptr = MAINICON_QT;
    for (int i = 0; i < 6; ++i)
    {
        uint32_t size = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += 4;

        QPixmap pm;
        pm.loadFromData(ptr, size);
        ret.addPixmap(pm);
        ptr += size;
    }

    return ret;
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    a.setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setWindowIcon(MakeAppIcon());

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(53,53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(255,255,255,120));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    a.setPalette(darkPalette);

    MainWindow w;
    w.show();
    return a.exec();
}
