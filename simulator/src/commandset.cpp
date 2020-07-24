#include "commandset.h"

const QStringList Command::reg_names = {
    "RA",
    "RB",
    "RC",
    "RD",
    "AR1",
    "AR2",
    "ASR",
    "HSP",
    "AP",
    "PC",
    "CR"
};

int CommandSet::to_int(const QString &str)
{
    bool ok;
    int res = str.toInt(&ok, 0);
    if (!ok)
        throw QString("Строка не является числом!");
    return res;
}

bool CommandSet::to_bool(const QString &str)
{
    if (str == "1" || str == "T" || str == "True")
        return 1;
    if (str == "0" || str == "F" || str == "False")
        return 0;
    throw QString("Строка не является логической константой!");
}

int CommandSet::get_regval(const QString & arg, const QVector<uint8_t>& regs)
{
    if (Command::reg_names.contains(arg))
        return regs.at(Command::reg_names.indexOf(arg));
    else
        return to_int(arg);

}

CommandSet::CommandSet()
{
    Command c;

    c = Command("MOV", {qMakePair(Command::RegVal, 7), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int y = Command::reg_names.indexOf(args.at(1));
        regs[y] = get_regval(args.at(0), regs);
        Q_UNUSED(memory);
    };
    c.structure = {1, 0, 0, 0, Command::I, Command::ARG1, Command::ARG2};
    commands.push_back(c);

    c = Command("ADD", {qMakePair(Command::Register, 4), qMakePair(Command::RegVal, 3), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int x = Command::reg_names.indexOf(args.at(0));
        int val = get_regval(args.at(1), regs);
        int z = Command::reg_names.indexOf(args.at(2));
        regs[z] = regs[x] + val;
        Q_UNUSED(memory);
    };
    c.structure = {1, 0, 1, 0, Command::I, Command::ARG1, Command::ARG2, Command::ARG3};
    commands.push_back(c);

    c = Command("SUB", {qMakePair(Command::Register, 4), qMakePair(Command::RegVal, 3), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int x = Command::reg_names.indexOf(args.at(0));
        int val = get_regval(args.at(1), regs);
        int z = Command::reg_names.indexOf(args.at(2));
        regs[z] = regs[x] - val;
        Q_UNUSED(memory);
    };
    c.structure = {1, 0, 1, 1, Command::I, Command::ARG1, Command::ARG2, Command::ARG3};
    commands.push_back(c);

    c = Command("LD", {qMakePair(Command::RegVal, 7), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int val = get_regval(args.at(0), regs);
        int dst = Command::reg_names.indexOf(args.at(1));
        regs[dst] = memory[val];
    };
    c.structure = {1, 1, 1, 0, Command::I, Command::ARG1, Command::ARG2};
    commands.push_back(c);

    c = Command("ST", {qMakePair(Command::RegVal, 7), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int val = get_regval(args.at(0), regs);
        int y = Command::reg_names.indexOf(args.at(1));
        memory[val] = regs[y];
    };
    c.structure = {1, 1, 0, 0, Command::I, Command::ARG1, Command::ARG2};
    commands.push_back(c);


    c = Command("HPUSH", {qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int v = get_regval(args.first(), regs);
        int hsp = Command::reg_names.indexOf("HSP");
        memory[regs[hsp]++] = v;
    };
    c.structure = {1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, Command::ARG1};
    commands.push_back(c);

    c = Command("HPOP",{qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int x = Command::reg_names.indexOf(args.at(0));
        int hsp = Command::reg_names.indexOf("HSP");
        regs[x] = memory[--regs[hsp]];
    };
    c.structure = {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, Command::ARG1};
    commands.push_back(c);

    c = Command("LDAW", {qMakePair(Command::Flag, 1), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int x = Command::reg_names.indexOf(args.at(1));
        int addr;
        if (args.at(1) == "AR1" || args.at(1) == "AR2")
            addr = ((regs[Command::reg_names.indexOf("ASR")] - regs[x]) % 64) * 2;
        else
            addr = (regs[x] % 64) * 2;
        int a, b;
        if (to_bool(args.at(0)))
        {
            a = Command::reg_names.indexOf("RC");
            b = Command::reg_names.indexOf("RD");
        }
        else
        {
            a = Command::reg_names.indexOf("RA");
            b = Command::reg_names.indexOf("RB");
        }
        if (addr < 0) addr = 128 + addr;
        regs[a] = memory.at(addr);
        regs[b] = memory.at(addr + 1);
    };
    c.structure = {0, 1, 1, 0,   0, 0, 0, 0,   0, 1, 1, Command::ARG1,   Command::ARG2};
    commands.push_back(c);

    c = Command("STAW", {qMakePair(Command::Flag, 1), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int x = Command::reg_names.indexOf(args.at(1));
        int addr;
        if (args.at(1) == "AR1" || args.at(1) == "AR2")
            addr = ((regs[Command::reg_names.indexOf("ASR")] - regs[x]) % 64) * 2;
        else
            addr = (regs[x] % 64) * 2;
        int a, b;
        if (to_bool(args.at(0)))
        {
            a = Command::reg_names.indexOf("RC");
            b = Command::reg_names.indexOf("RD");
        }
        else
        {
            a = Command::reg_names.indexOf("RA");
            b = Command::reg_names.indexOf("RB");
        }
        if (addr < 0) addr = 128 + addr;
        memory[addr] = regs.at(a);
        memory[addr + 1] = regs.at(b);
    };
    c.structure = {0, 1, 0, 0,   0, 0, 0, 0,   0, 1, 1, Command::ARG1,   Command::ARG2};
    commands.push_back(c);

    c = Command("STEP", {qMakePair(Command::Flag, 1), qMakePair(Command::Flag, 1)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int x;
        if (to_bool(args.at(0)))
            x = Command::reg_names.indexOf("AP");
        else
            x = Command::reg_names.indexOf("HSP");
        if (!to_bool(args.at(1)))
            regs[x]++;
        else
            regs[x]--;
        Q_UNUSED(memory);
    };
    c.structure = {0, 1, Command::ARG2, 1,   0, 0, 0, 0,   0, 0, 0, Command::ARG1,   0, 0, 0, 0};
    commands.push_back(c);

    c = Command("B", {qMakePair(Command::Value, 7)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int a = to_int(args.at(0));
        regs[Command::reg_names.indexOf("PC")] = a;
        Q_UNUSED(memory);
    };
    c.structure = {0, 0, 1, 0,   1, Command::ARG1,   0, 0, 0, 0};
    commands.push_back(c);

    c = Command("BL2", {qMakePair(Command::Value, 7)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int a = to_int(args.at(0));
        int ra = regs[Command::reg_names.indexOf("RA")];
        if (ra < 2)
            regs[Command::reg_names.indexOf("PC")] = a;
        Q_UNUSED(memory);
    };
    c.structure = {0, 0, 1, 0,   1, Command::ARG1,   0, 0, 1, 0};
    commands.push_back(c);

    c = Command("BCMP", {qMakePair(Command::Value, 7)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int a = to_int(args.at(0));
        int ra = regs[Command::reg_names.indexOf("RA")];
        int rb = regs[Command::reg_names.indexOf("RB")];
        int rc = regs[Command::reg_names.indexOf("RC")];
        int rd = regs[Command::reg_names.indexOf("RD")];
        int x, y;
        if (regs[Command::reg_names.indexOf("CR")] & 1)
        {
            x = ra + (rb << 8);
            y = rc + (rd << 8);
        }
        else
        {
            x = rc + (rd << 8);
            y = ra + (rb << 8);
        }
        if (x < y)
            regs[Command::reg_names.indexOf("PC")] = a;
        Q_UNUSED(memory);
    };
    c.structure = {0, 0, 1, 0,   1, Command::ARG1,   0, 0, 1, 1};
    commands.push_back(c);

    c = Command("BEOA", {qMakePair(Command::Value, 7)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int a = to_int(args.at(0));
        int ap = regs[Command::reg_names.indexOf("AP")];
        int aep = regs[Command::reg_names.indexOf("CR")];
        if (ap == aep)
            regs[Command::reg_names.indexOf("PC")] = a;
        Q_UNUSED(memory);
    };
    c.structure = {0, 0, 1, 0,   1, Command::ARG1,   0, 0, 0, 1};
    commands.push_back(c);

    c = Command("L", {qMakePair(Command::RegVal, 7), qMakePair(Command::Register, 4)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int val = get_regval(args.at(0), regs);
        int y = Command::reg_names.indexOf(args.at(1));
        uint8_t L[] = {1, 1, 3, 5, 9, 15, 25, 41};
        regs[y] = L[val];
        Q_UNUSED(memory);
    };
    c.structure = {1, 0, 0, 1, Command::I, Command::ARG1, Command::ARG2};
    commands.push_back(c);

    c = Command("LINK", {qMakePair(Command::Value, 7)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        int a = to_int(args.at(0));
        memory[96] = a;
        Q_UNUSED(regs);
    };
    c.structure = {0, 0, 1, 1,   1, Command::ARG1,   0, 0, 0, 0};
    commands.push_back(c);

    c = Command("RET", {});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
        regs[Command::reg_names.indexOf("PC")] = memory[96];
        Q_UNUSED(args);
    };
    c.structure = {0, 0, 0, 1,   1, 1, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0};
    commands.push_back(c);


    c = Command("HLT", {});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
      Q_UNUSED(args); Q_UNUSED(memory); Q_UNUSED(regs);
    };
    c.structure = {0, 0, 0, 1,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0};
    commands.push_back(c);

    c = Command("OUT", {qMakePair(Command::Flag, 1)});
    c.operation = [](const QStringList& args, QVector<uint8_t>& memory, QVector<uint8_t>& regs)
    {
      Q_UNUSED(args); Q_UNUSED(memory); Q_UNUSED(regs);
    };
    c.structure = {0, 0, 0, 0,   1, 0, 0, 0,   0, 0, 0, Command::ARG1,   0, 0, 0, 0};
    commands.push_back(c);
}

QString CommandSet::check(const QString &str) const
{
    QStringList tokens = str.split(QRegExp("\\s+"));
    for(auto i = tokens.begin(); i != tokens.end(); )
        if (i->isEmpty())
            i = tokens.erase(i);
        else
            ++i;
    qDebug() << "check:" << str << tokens;
    if (tokens.size())
    {
        QString name = tokens.at(0);
        QVector<QPair<Command::OperandType, int>> syntax;
        bool ok = false;
        for(Command c : commands) if (c.name == name)
        {
            ok = true;
            syntax = c.syntax;
            break;
        }
        if (!ok)
            return QString("Неизвестная комманда: %1").arg(name);
        if (tokens.size() != syntax.size() + 1)
            return QString("Неверное количество аргументов");
        for (int i = 0; i < syntax.size(); i++)
        {
            switch (syntax.at(i).first) {
            case Command::Register:
                if (!Command::reg_names.contains(tokens.at(i+1)))
                    return QString("Неизвестное имя регистра: %1").arg(tokens.at(i+1));
                if (Command::reg_names.indexOf(tokens.at(i+1)) >= (1 << syntax.at(i).second))
                    return QString("Регистр не может быть использован в этой команде: %1").arg(tokens.at(i+1));
                break;
            case Command::Value:
            {
                int v;
                try {
                    v = to_int(tokens.at(i + 1));
                }
                catch(QString s)
                {
                    return s;
                }
                if (v >= (1 << syntax.at(i).second))
                    return QString("Операнд слишком большой: %1").arg(tokens.at(i+1));
                break;
            }
            case Command::Flag:
                try {
                    to_bool(tokens.at(i + 1));
                }
                catch(QString s)
                {
                    return s;
                }
                break;
            case Command::RegVal:
                if (Command::reg_names.contains(tokens.at(i+1)))
                {
                    if (Command::reg_names.indexOf(tokens.at(i+1)) >= (1 << syntax.at(i).second))
                        return QString("Регистр не может быть использован в этой команде: %1").arg(tokens.at(i+1));
                }
                else
                {
                    int v;
                    try {
                        v = to_int(tokens.at(i + 1));
                    }
                    catch(QString)
                    {
                        return QString("Неподходящий операнд: %1").arg(tokens.at(i+1));
                    }
                    if (v >= (1 << syntax.at(i).second))
                        return QString("Операнд слишком большой: %1").arg(tokens.at(i+1));
            }
                break;
            default:
                break;
            }
        }
    }
    return QString();
}

void CommandSet::exec(const QString &str, QVector<uint8_t> &memory, QVector<uint8_t> &regs)
{
    QString checkres = check(str);
    if (checkres.size())
        throw checkres;
    QStringList tokens = str.split(QRegExp("\\s+"));
    QStringList args;
    for (auto i = tokens.begin() + 1; i != tokens.end(); ++i)
        args << *i;
    //qDebug() << "Executing:" << str;
    for(Command c : commands)
        if (c.name == tokens.at(0))
        {
            c.operation(args, memory, regs);
            //qDebug() << "Executed";
            return;
        }
}

int CommandSet::code(const QString &str) const
{
    QStringList tokens = str.split(QRegExp("\\s+"));
    int code = 0;
    for (Command c : commands)
        if (c.name == tokens.first())
        {
            int count = 0;
            for (auto it = c.structure.begin(); it != c.structure.end(); it++)
            {
                count++;
                if (*it == 0)
                    continue;
                if (*it == 1)
                {
                    code |= 1 << (16 - count);
                    continue;
                }
                if (*it == Command::I)
                {
                    int j = 0;
                    while(c.syntax.at(j).first != Command::RegVal) j++;
                    if (!Command::reg_names.contains(tokens.at(j+1)))
                        code |= 1 << (16 - count);
                }
                if (*it >= Command::ARG1)
                {
                    int n = *it - Command::ARG1;
                    count += c.syntax.at(n).second - 1;
                    switch (c.syntax.at(n).first) {
                    case Command::Value:
                        code |= to_int(tokens.at(n+1)) << (16 - count);
                        break;
                    case Command::Register:
                        if (c.name == "STAW" || c.name == "LDAW")
                        {
                            int reg = 14;
                            if (tokens.at(n+1) == "AR2")
                                reg++;
                            code |= reg << (16 - count);
                        }
                        else
                            code |= Command::reg_names.indexOf(tokens.at(n+1)) << (16 - count);
                        break;
                    case Command::RegVal:
                        if (Command::reg_names.contains(tokens.at(n+1)))
                            code |= Command::reg_names.indexOf(tokens.at(n+1)) << (16 - count);
                        else
                            code |= to_int(tokens.at(n+1)) << (16 - count);
                        break;
                    case Command::Flag:
                        code |= static_cast<int>(to_bool(tokens.at(n+1))) << (16 - count);
                        break;
                    }
                }
            }
        }
    qDebug() << "coding:" << str << " code:" << code;
    return code;
}
