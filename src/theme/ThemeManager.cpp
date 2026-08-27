#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QPalette>
#include <QStyle>
#include <QWidget>

// Singleton
ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager()
    : QObject(nullptr)
    , m_currentTheme(Light)
{
}

void ThemeManager::initialize(Theme initial)
{
    m_currentTheme = initial;
    applyStyleSheet();
    applyPalette();
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }

    m_currentTheme = theme;

    if (applyStyleSheet()) {
        applyPalette();
        emit themeChanged(theme);
    }
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

bool ThemeManager::applyStyleSheet()
{
    const QString path = themePath() + QStringLiteral("style.qss");
    QString content;

    if (!loadStyleSheet(path, content)) {
        return false;
    }

    qApp->setStyleSheet(content);
    return true;
}

bool ThemeManager::loadStyleSheet(const QString& path, QString& outContent)
{
    QFile file(path);
    if (!file.exists()) {
        m_lastError = QStringLiteral("Theme QSS file not found: %1").arg(path);
        qWarning("ThemeManager: %s", qPrintable(m_lastError));
        return false;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        m_lastError = QStringLiteral("Cannot open theme QSS file: %1 (%2)")
                          .arg(path, file.errorString());
        qWarning("ThemeManager: %s", qPrintable(m_lastError));
        return false;
    }

    outContent = QString::fromUtf8(file.readAll());
    file.close();

    if (outContent.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("Theme QSS file is empty: %1").arg(path);
        qWarning("ThemeManager: %s", qPrintable(m_lastError));
        return false;
    }

    m_lastError.clear();
    return true;
}

void ThemeManager::refreshWidget(QWidget* widget)
{
    if (!widget) return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

void ThemeManager::applyPalette()
{
    // Set a minimal QPalette to support colors not controlled by QSS
    // (e.g., QPlainTextEdit selection, QTableView grid hints).
    QPalette palette = qApp->palette();

    if (m_currentTheme == Dark) {
        palette.setColor(QPalette::Window,          QColor(0x1E, 0x1E, 0x1E));
        palette.setColor(QPalette::WindowText,      QColor(0xAD, 0xBA, 0xC7));
        palette.setColor(QPalette::Base,            QColor(0x22, 0x27, 0x2E));
        palette.setColor(QPalette::AlternateBase,   QColor(0x25, 0x2A, 0x31));
        palette.setColor(QPalette::Text,            QColor(0xAD, 0xBA, 0xC7));
        palette.setColor(QPalette::Button,          QColor(0x37, 0x3E, 0x47));
        palette.setColor(QPalette::ButtonText,      QColor(0xAD, 0xBA, 0xC7));
        palette.setColor(QPalette::Highlight,       QColor(0x3A, 0x42, 0x4A));
        palette.setColor(QPalette::HighlightedText, QColor(0xAD, 0xBA, 0xC7));
    } else {
        palette.setColor(QPalette::Window,          QColor(0xF0, 0xF0, 0xF0));
        palette.setColor(QPalette::WindowText,      QColor(0x00, 0x00, 0x00));
        palette.setColor(QPalette::Base,            QColor(0xFF, 0xFF, 0xFF));
        palette.setColor(QPalette::AlternateBase,   QColor(0xF8, 0xF8, 0xF8));
        palette.setColor(QPalette::Text,            QColor(0x00, 0x00, 0x00));
        palette.setColor(QPalette::Button,          QColor(0xF0, 0xF0, 0xF0));
        palette.setColor(QPalette::ButtonText,      QColor(0x2A, 0x2A, 0x2A));
        palette.setColor(QPalette::Highlight,       QColor(0xF0, 0xFA, 0xDE));
        palette.setColor(QPalette::HighlightedText, QColor(0x00, 0x00, 0x00));
    }

    qApp->setPalette(palette);
}

QString ThemeManager::themePath() const
{
    return m_currentTheme == Dark
        ? QStringLiteral(":/themes/dark/")
        : QStringLiteral(":/themes/light/");
}

QString ThemeManager::lastError() const
{
    return m_lastError;
}
