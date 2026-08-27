#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        Light = 0,
        Dark  = 1
    };
    Q_ENUM(Theme)

    static ThemeManager* instance();

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    // Initialize with default theme (Light). Call once after QApplication is created.
    void initialize(Theme initial = Light);

    // Switch to a different theme. Reloads QSS and emits themeChanged.
    void setTheme(Theme theme);
    Theme currentTheme() const;

    // Apply the global QSS stylesheet to qApp. Returns false on failure.
    bool applyStyleSheet();

    // Refresh a widget's style after changing a dynamic property.
    // Call after setProperty() to trigger QSS re-evaluation.
    static void refreshWidget(QWidget* widget);

    // Load and apply QApplication palette (optional, for non-QSS-settable colors).
    void applyPalette();

    // Returns the resource path prefix for the current theme (e.g. ":/themes/light/").
    QString themePath() const;

    // Returns the last error message (empty if no error).
    QString lastError() const;

signals:
    void themeChanged(Theme theme);

private:
    ThemeManager();
    ~ThemeManager() override = default;

    bool loadStyleSheet(const QString& path, QString& outContent);

    static ThemeManager* s_instance;

    Theme m_currentTheme = Light;
    QString m_lastError;
};

#endif // THEMEMANAGER_H
