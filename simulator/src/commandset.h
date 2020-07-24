#ifndef COMMANDSET_H
#define COMMANDSET_H

#include <QStringList>
#include <QVector>
#include <QPair>
#include <QDebug>

struct Command
{
    enum OperandType{
        Register,
        Value,
        Flag,
        RegVal
    };
    enum Bit{
        Zero,
        One,
        I,
        ARG1,
        ARG2,
        ARG3
    };

    static const QStringList reg_names;
    QString name;
    //второй параметр -- разрядность в битах
    QVector<QPair<OperandType, int>> syntax;
    QVector<int> structure;
    void (*operation)(const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs);
    Command():
        name(), syntax(), structure(), operation(nullptr) {}
    Command(const QString& Name, QVector<QPair<OperandType, int>> Syntax):
        name(Name), syntax(Syntax), structure(), operation(nullptr) {}
};


class CommandSet
{
    QVector<Command> commands;
    static int to_int(const QString&);
    static bool to_bool(const QString&);
    static int get_regval(const QString&, const QVector<uint8_t>&);
public:
    CommandSet();
    //проверяет комманду на синтаксис, возвращает ошибку или пустую строку, если ошибок нет
    QString check(const QString& str) const;
    void exec(const QString& str, QVector<uint8_t>& memory, QVector<uint8_t>& regs);
    int code(const QString& str) const;
};

#endif // COMMANDSET_H
