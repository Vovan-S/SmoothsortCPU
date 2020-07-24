#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QWidget>
#include <QSplitter>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QMap>
#include <QMultiMap>
#include <QFontMetrics>
#include <QSize>
#include <QDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QProgressDialog>

#include "commandset.h"


class Simulator : public QWidget
{
    Q_OBJECT

/****************Элементы управления****************/

    //поле для ввода программы
    //QTextEdit *p_te;
    //таблица значений регистров
    QTableWidget *p_tbreg;
    //таблица значений памяти
    QTableWidget *p_tbmem;
    //таблица ввода программы
    QTableWidget *p_tbprog;
    //ошибки
    QTableWidget *p_tberr;

    //кнопки для отладки
    //начать / отстановить отладку
    QPushButton *p_btdebug;
    //следующая команда
    QPushButton *p_btstep;
    //следующая точка останова
    QPushButton *p_btnext;

    //кнопки для редактирования
    //вставить строчку
    QPushButton *p_btinsert;
    //удалить строчку
    QPushButton *p_btdeleteline;
    //скопировать содержимое
    QPushButton *p_btcopy;
    //вставить содержимое
    QPushButton *p_btpaste;
    //добавить точку останова
    QPushButton *p_btaddbp;
    //загрузить данные инициализации памяти и регистров
    QPushButton *p_btinitmem;

    //управление файлом
    //новый файл
    QPushButton *p_btnew;
    //открыть файл
    QPushButton *p_btopen;
    //сохранить файл
    QPushButton *p_btsave;
    //сохранить файл как
    QPushButton *p_btsaveas;

    //тест сортировок
    QPushButton *p_bttest;
    QPushButton *p_gencode;

    //размещение в виджете
    //для кнопок отладки
    QHBoxLayout *p_ltbtdebug;
    //для кнопок редактирования
    QHBoxLayout *p_ltbtedit;
    //для кнопок файла
    QHBoxLayout *p_ltfile;
    //основной лэйаут
    QVBoxLayout *p_ltmain;
    //вертикальный разделитель для таблицы регистров и других таблиц
    QSplitter *p_spreg;
    //горизонтальный разделитель для таблицы ассемблера и памяти
    QSplitter *p_sperr;


/****************Переменные приложения****************/

    QString filename;
    bool saved;
    //текущий статус
    enum State{
        Editing,    //пользователь редактирует код
        Breakpoint,    //приложение выполняет обработку и стоит на брейкпойнте
        Calculating //приложение выполняет обработку
    } state;
    QString buffer;


/****************Симулятор процессора****************/
    //рабочая память
    QVector<uint8_t> memory;
    //регистры
    QVector<uint8_t> registers;
    //набор команд
    CommandSet cset;

/****************Отладка****************/
    //точки останова
    QVector<bool> breakpoints;
    //ошибки <строка, <тип, сообщение>>
    QMultiMap<int, QPair<int, QString>> errors;
    //метки <имя, адрес>
    QMap<QString, int> marks;
    //неразрешенные метки <имя, строки с упоминанием>
    QMap<QString, QSet<int>> unresolved;
    //ошибки с метками
    QStringList mark_errors;

    bool marking_line;
    bool pass_one_breakpoint;

private slots:
    void update_registers();
    void update_memory();
    void update_errors();
    void update_editing_field();
    void update_title();

    void program_edited(int row, int column);
    void file_button();
    void saveas();
    void save();
    void edit_buttons();

    void lock_editing_buttons();

    void execute();

    void regtb_change(int row, int column);

    void init_mem();

    void test();

    void gen_code();

private:
    void update_marks();
    void update_breakpoints();
    bool warn_unsaved();

    void save(QString filename);

    void init();

    void setState(State);

    int execute_one(bool f = true);

    int PC();

    void mark_line();
    void fix_prog_table();
    void substitute_mark(QString&);
    //TODO: редактирование таблицы

public:
    Simulator(QWidget *parent = nullptr);
    ~Simulator();
};
#endif // SIMULATOR_H
