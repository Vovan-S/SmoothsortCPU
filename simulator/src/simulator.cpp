#include "simulator.h"

void Simulator::update_registers()
{
    for (int i = 0; i < registers.size(); i++)
    {
        auto item = p_tbreg->item(i, 0);
        if (item == nullptr)
        {
            item = new QTableWidgetItem;
            //item->setData(0, registers.at(i));
            item->setText("0x" + QString::number(registers.at(i), 16));
            p_tbreg->setItem(i, 0, item);
            item->setFlags(Qt::ItemIsEnabled);
            QFontMetrics fm(item->font());
            item->setSizeHint(QSize(fm.size(0, "255")));
        }
        QBrush b(Qt::white);
        //qDebug() << Command::reg_names.at(i)
        //         << "\ttable value: " << item->text().toInt(nullptr, 0)
        //         << " register value: " << registers.at(i);
        if (item->text().toInt(nullptr, 0) != registers.at(i))
        {
            b = QBrush(Qt::blue);
            //qDebug() << "\told data:" << item->text().toInt(nullptr, 0) << "new data: " << registers.at(i);
            //item->setData(0, registers.at(i));
            item->setText("0x" + QString::number(registers.at(i), 16));
        }
        item->setBackground(b);
    }
    p_tbreg->resizeColumnsToContents();
}

void Simulator::update_memory()
{
    for (int i = 0; i < p_tbmem->rowCount(); i++)
        for (int j = 0; j < p_tbmem->columnCount(); j++)
        {
            int val = memory.at(i * p_tbmem->columnCount() + j);
            auto item = p_tbmem->item(i, j);
            if (item == nullptr)
            {
                item = new QTableWidgetItem("0x" + QString::number(val, 16));
                //item->setData(0, val);
                item->setFlags(Qt::ItemIsEnabled);
                p_tbmem->setItem(i, j, item);
                QFontMetrics fm(item->font());
                item->setSizeHint(QSize(fm.size(0, "255")));
            }
            QBrush b(Qt::white);
            if (item->text().toInt(nullptr, 16) != val)
            {
                b = QBrush(Qt::blue);
                //item->setData(0, val);
                item->setText("0x" + QString::number(val, 16));
            }
            item->setBackground(b);
        }
    p_tbmem->resizeColumnsToContents();
}

void Simulator::update_errors()
{
    p_tberr->clearContents();
    if (errors.empty() && mark_errors.empty())
    {
        p_tberr->setRowCount(1);
        QTableWidgetItem *item = new QTableWidgetItem;
        p_tberr->setItem(0, 0, item);
        item = new QTableWidgetItem;
        p_tberr->setItem(0, 1, item);
        item = new QTableWidgetItem("Ошибок нет");
        p_tberr->setItem(0, 2, item);
    }
    else
    {
        p_tberr->setRowCount(errors.size() + mark_errors.size());
        int c = 0;
        for(auto i = errors.begin(); i != errors.end(); i++)
        {
            QTableWidgetItem *type = new QTableWidgetItem((i.value().first == 0)? "W" : "E");
            QTableWidgetItem *line = new QTableWidgetItem("0x" + QString::number(i.key(), 16));
            QTableWidgetItem *mes = new QTableWidgetItem(i.value().second);
            p_tberr->setItem(c, 0, type);
            p_tberr->setItem(c, 1, line);
            p_tberr->setItem(c, 2, mes);
            c++;
        }
        for (auto i = mark_errors.begin(); i != mark_errors.end(); i++)
        {
            QTableWidgetItem *type = new QTableWidgetItem("M");
            QTableWidgetItem *line = new QTableWidgetItem;
            QTableWidgetItem *mes = new QTableWidgetItem(*i);
            p_tberr->setItem(c, 0, type);
            p_tberr->setItem(c, 1, line);
            p_tberr->setItem(c, 2, mes);
            c++;
        }
    }
    for (int r = 0; r < p_tberr->rowCount(); r++)
        for (int c = 0; c < p_tberr->columnCount(); c++)
        {
            auto item = p_tberr->item(r, c);
            if (item != nullptr)
                item->setFlags(Qt::ItemIsEnabled);
        }
}

void Simulator::update_editing_field()
{
    p_tbprog->clearSelection();
    for (int r = 0; r < p_tbprog->rowCount(); r++)
    {
        auto bp = p_tbprog->item(r, 0);
        if (bp == nullptr)
        {
            bp = new QTableWidgetItem;
            bp->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            bp->setCheckState(Qt::Unchecked);
            p_tbprog->setItem(r, 0, bp);
        }
        auto mark = p_tbprog->item(r, 1);
        if (mark == nullptr)
        {
            mark = new QTableWidgetItem;
            p_tbprog->setItem(r, 1, mark);
        }
        if (state == Simulator::Editing)
            mark->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        else
            mark->setFlags(Qt::ItemIsEnabled);
        auto command = p_tbprog->item(r, 2);
        if (command == nullptr)
        {
            command = new QTableWidgetItem;
            p_tbprog->setItem(r, 2, command);
        }
        if (state == Simulator::Editing)
            mark->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        else
            mark->setFlags(Qt::ItemIsEnabled);
        command->setFlags(mark->flags());
    }
}

void Simulator::update_title()
{
    QString title = filename;
    if (title.contains('/'))
        for (int i = 1; i <= title.size(); i++)
            if (title.at(title.size() - i) == '/')
            {
                title = title.right(i - 1);
            }
    if (!saved)
        title += '*';
    switch (state) {
    case Editing:
        title += " - Редактирование - ";
        break;
    case Breakpoint:
        title += " - Остановлен - ";
        break;
    case Calculating:
        title += " - Вычисление - ";
        break;
    }
    title += "Симулятор";
    setWindowTitle(title);
}

void Simulator::program_edited(int row, int column)
{
    if (marking_line)
        return;
    //qDebug() << "table edited" << row << column;
    auto item = p_tbprog->item(row, column);
    if (item == nullptr)
        return;
    if (column == 0)
    {
        if (item->checkState() == Qt::Checked)
            breakpoints[column] = true;
        else
            breakpoints[column] = false;
    }
    if (column == 1)
    {
        update_marks();
        if (saved == true)
        {
            saved = false;
            update_title();
        }
    }
    if (column == 2)
    {
        if (saved == true)
        {
            saved = false;
            update_title();
        }
        for(auto i = errors.lowerBound(row); i != errors.upperBound(row); )
            if (i.value().first != 2)
                i = errors.erase(i);
            else
                ++i;
        for(auto i = unresolved.begin(); i != unresolved.end(); i++)
            i->remove(row);
        QString command = item->text();
        //qDebug() << "command" << command;
        if (command.isEmpty())
        {
            update_errors();
            return;
        }
        if(command.contains('@'))
        {
            int begin = command.indexOf('@');
            int end = command.indexOf(' ', begin);
            QString mark = command.mid(begin + 1, end);
            qDebug() << "mark" << mark;
            command.replace('@' + mark, "0");
            unresolved[mark].insert(row);
            update_marks();
        }
        QString error = cset.check(command);
        if (!error.isEmpty())
            errors.insert(row, qMakePair(1, error));
    }
    update_errors();
}

void Simulator::file_button()
{
    if (sender() == p_btnew)
        if (warn_unsaved())
            init();
    if (sender() == p_btopen)
        if (warn_unsaved())
        {
            QString fname = QFileDialog::getOpenFileName(this,
                                                         "Открыть файл");
            QStringList program;
            QVector<uint8_t> regs(registers.size());
            QVector<uint8_t> mem(memory.size());
            QFile f(fname);
            bool ok = true;
            if (f.open(QFile::ReadOnly))
            {
                QTextStream in(&f);
                int c = 0;
                while (!in.atEnd())
                {
                    if (c < 128)
                    {
                        program << in.readLine();
                        c++;
                        continue;
                    }
                    try {
                        for (int i = 0; i < regs.size(); i++)
                        {
                            int x;
                            in >> x;
                            regs[i] = x;
                        }
                        for (int i = 0; i < mem.size(); i++)
                        {
                            int x;
                            in >> x;
                            mem[i] = x;
                        }
                    } catch (...) {
                        ok = false;
                    }
                }
                if (!ok || c < 128)
                {
                    QMessageBox::critical(this,
                                          "Ошибка!",
                                          "Не удалось считать файл!");
                    return;
                }
                init();
                registers.swap(regs);
                memory.swap(mem);
                for (int r = 0; r < 128; r++)
                {
                    QString code;
                    if (program.at(r).endsWith('\n'))
                        program[r].chop(1);
                    if(program.at(r).startsWith('@'))
                    {
                        int index = program.at(r).indexOf(' ');
                        QString mark = program.at(r).mid(1, index - 1);
                        code = program.at(r).mid(index + 1);
                        p_tbprog->item(r, 1)->setText(mark);
                    }
                    else
                        code = program.at(r);
                    p_tbprog->item(r, 2)->setText(code);
                }
                update_marks();
                update_errors();
                update_registers();
                update_memory();
                filename = fname;
                saved = true;
                update_title();
            }
        }
}

void Simulator::saveas()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                 "Сохранить как...");
    if (fname.isEmpty())
        return;
    filename = fname;
    save(filename);
    saved = true;
    update_title();
}

void Simulator::save()
{
    if (filename == "Новый файл")
    {
        QString fname = QFileDialog::getSaveFileName(this,
                                                     "Сохранить");
        if (fname.isEmpty())
            return;
        filename = fname;
    }
    save(filename);
    saved = true;
    update_title();
}

void Simulator::edit_buttons()
{
    auto selection = p_tbprog->selectedItems();
    if (selection.isEmpty())
        return;
    auto item = selection.first();
    if (sender() == p_btcopy)
    {
        buffer = item->text();
    }
    if (sender() == p_btpaste)
    {
        if (!buffer.isEmpty())
            item->setText(buffer);
    }
    if (sender() == p_btinsert)
    {
        p_tbprog->insertRow(item->row());
        fix_prog_table();
    }
    if (sender() == p_btdeleteline)
    {
        p_tbprog->removeRow(item->row());
        fix_prog_table();
    }
}

void Simulator::lock_editing_buttons()
{
    auto buttons = {p_btcopy, p_btpaste, p_btinsert, p_btdeleteline};
    if (p_tbprog->selectedItems().size() == 1)
        for (auto button: buttons)
            button->setEnabled(true);
    else
        for (auto button: buttons)
            button->setEnabled(false);
}

void Simulator::execute()
{
    update_breakpoints();
    switch (state) {
    case Editing:
    {
        setState(Calculating);
        pass_one_breakpoint = false;
        execute();
        break;
    }
    case Calculating:
    {
        //qDebug() << breakpoints;
        long watchDog = 0;
        long watchDogMax = 20000;
        int res = -2;
        //qDebug() << "Calculating:: line:" << PC() << " breakpoint:" << breakpoints.at(PC());
        while(pass_one_breakpoint || !breakpoints.at(PC()))
        {
            if (pass_one_breakpoint)
                pass_one_breakpoint = false;
            if (++watchDog > watchDogMax)
            {
                res = -1;
                break;
            }
            res = execute_one();
            if (res != 1)
                break;
            res = -2;
        }
        switch (res) {
        case -2:
            setState(Breakpoint);
            update_registers();
            update_memory();
            return;
        case -1:
        {
            QMessageBox::StandardButton res =
                    QMessageBox::warning(this,
                                         "Сработал сторожевой таймер!",
                                         "Продолжить выполнение?",
                                         QMessageBox::Ok | QMessageBox::No,
                                         QMessageBox::No);
            if (res == QMessageBox::Ok)
                execute();
            else
                setState(Editing);
            return;
        }
        case 2:
            update_registers();
            update_memory();
            QMessageBox::information(this,
                                     "Успешно выполнено",
                                     "Выполнение успешно завершено");
            setState(Editing);
            break;
        case 0:
            setState(Editing);
            break;
        }
        break;
    }
    case Breakpoint:
    {
        if (sender() == p_btnext)
        {
            setState(Calculating);
            pass_one_breakpoint = true;
            execute();
        }
        else if (sender() == p_btstep)
        {
            int res = execute_one();
            mark_line();
            update_registers();
            update_memory();
            if (res == 0)
            {
                setState(Editing);
            }
            if (res == 2)
            {
                update_registers();
                update_memory();
                QMessageBox::information(this,
                                         "Успешно выполнено",
                                         "Выполнение успешно завершено");
                setState(Editing);
            }
        }
        else
        {
            setState(Editing);
        }
        return;
    }
    }
}

void Simulator::regtb_change(int row, int column)
{
    if (row >= 0)
        qDebug() << "changed:" << Command::reg_names.at(row) << p_tbreg->item(row, column)->text();
}

void Simulator::init_mem()
{
    QString fname = QFileDialog::getOpenFileName(this,
                                                    "Открыть файл инициализации");
    QVector<uint8_t> regs(registers.size());
    QVector<uint8_t> mem(memory.size());
    QFile f(fname);
    bool ok = true;
    if (f.open(QFile::ReadOnly))
    {
        QTextStream in(&f);
        try {
             for (int i = 0; i < regs.size(); i++)
             {
                    int x;
                    in >> x;
                    regs[i] = x;
                }
                for (int i = 0; i < mem.size(); i++)
                {
                    int x;
                    in >> x;
                    mem[i] = x;
                }
            } catch (...) {
                ok = false;
            }
    }
    else
        ok = false;
    if (!ok)
    {
        QMessageBox::critical(this,
                              "Ошибка!",
                              "Не удалось считать файл!");
        return;
    }
    registers.swap(regs);
    memory.swap(mem);
    update_registers();
    update_memory();
}

void Simulator::test()
{
    QString fname = QFileDialog::getOpenFileName(this,
                                                 "Выберите файл с массивами для тестов");
    QFile f(fname);
    QVector<QVector<int>> arrays;
    if (f.open(QFile::ReadOnly))
    {
        QTextStream in(&f);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            qDebug() << "Line read: " << line;
            QTextStream reader(&line);
            arrays.push_back(QVector<int>());
            while(!reader.atEnd())
            {
                int x;
                reader >> x;
                arrays.back().push_back(x);
            }
        }
        f.close();
    }
    else
    {
        QMessageBox::critical(this,
                              "Ошибка чтения файла!",
                              "Не удалось отрыть файл!");
        return;
    }
    QStringList testlog;
    int successfull = 0;
    int sorted_with_errors = 0;
    int crashed = 0;
    int watchDogActivated = 0;
    QProgressDialog progress("Тестирование сортировки",
                             "Прервать тестирование",
                             0,
                             arrays.size(),
                             this);
    progress.setWindowModality(Qt::WindowModal);
    for (auto ar: arrays)
    {
        if (progress.wasCanceled())
            break;
        if (ar.isEmpty())
            continue;
        //инициализация регистров
        registers.fill(0);
        registers[7] = 80;
        int size = std::min(40, ar.size());
        registers[9] = size;
        //инициализация памяти
        memory.fill(0);
        for (int i = 0; i < size; i++)
        {
            memory[2 * i] = ar.at(i);
            memory[2 * i + 1] = ar.at(i) >> 8;
        }
        int inversions = 0;
        for (int i = 0; i < size - 1; i++)
            for (int j = i + 1; j < size; j++)
                if (ar.at(i) > ar.at(j))
                    inversions++;
        QString log = QString("\nМассив №%1. Размер: %2. Инверсий: %3\n")
                .arg(testlog.size() + 1)
                .arg(size)
                .arg(inversions);
        for (int i = 0; i < size; i++)
            log += QString::number(ar.at(i)) + ' ';
        int commands = 0;
        int watchDogMax = 200000;
        int res = 0;
        do
        {
            res = execute_one();
            if (++commands > watchDogMax)
            {
                res = -1;
                break;
            }
        }
        while (res == 1);
        switch (res) {
        case 2:
        {
            bool sorted = true;
            int last = -1;
            for (int i = 0; i < size; i++)
            {
                int x = memory.at(2*i) + (memory.at(2*i+1) << 8);
                if (last > x)
                {
                    sorted = false;
                    break;
                }
                last = x;
            }
            if (sorted)
            {
                successfull++;
                log += "\nУспешно отсортировано:\n";
            }
            else
            {
                sorted_with_errors++;
                log += "\nОтсортировано с ошибками:\n";
            }
            for (int i = 0; i < size; i++)
                log += QString::number(memory.at(2*i) + (memory.at(2*i+1) << 8)) + ' ';
            log += '\n';
            break;
        }
        case 0:
            crashed++;
            log += "\nПроизошла ошибка.\n";
            break;
        case -1:
            watchDogActivated++;
            log += "\nАктивирован сторожевой таймер.\n";
            break;
        }
        log += QString("Выполнено команд: %1\n").arg(commands);
        testlog << log;
        progress.setValue(testlog.size());
    }
    QFile fout(fname + ".log");
    fout.open(QFile::WriteOnly);
    QTextStream out(&fout);
    out << tr("Протестировано массивов: ") << testlog.size() << '\n';
    out << tr("Успешно отсортировано: ") << successfull << '\n';
    out << tr("Отсортировано с ошибками: ") << sorted_with_errors << '\n';
    out << tr("Произошла ошибка: ") << crashed << '\n';
    out << tr("Сторожевой таймер активирован: ") << watchDogActivated << '\n';
    out << testlog.join("\n");
    fout.close();
    QMessageBox::information(this,
                             "Тестирование завершено",
                             "Тестирование завершено, результат сохранен в файле '" + fname + ".log'.");
}

void Simulator::gen_code()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                 "Сгенерировать код...",
                                                 "I:\\Uni\\Дистанционное обучение\\СК\\Суперкомпы\\logicim");
    QFile f(fname);
    if (f.open(QFile::WriteOnly))
    {
        QTextStream out(&f);
        out << "v2.0 raw\n";
        for (int r = 0; r < p_tbprog->rowCount(); r++)
        {
            QString command = p_tbprog->item(r, 2)->text();
            QString code;
            if (command.isEmpty())
                code = "00";
            else {
                substitute_mark(command);
                code = QString::number(cset.code(command), 16);
            }
            out << code << ' ';
        }
        f.close();
    }
}

void Simulator::update_marks()
{
    marks.clear();
    mark_errors.clear();
    QSet<QString> reported;
    for (int r = 0; r < p_tbprog->rowCount(); r++)
    {
        auto item = p_tbprog->item(r, 1);
        if (item == nullptr)
            return;
        QString mark = item->text();
        if (mark.isEmpty())
            continue;
        if (marks.contains(mark))
        {
            if(!reported.contains(mark))
            {
                mark_errors << QString("Повтроное использование метки: %1").arg(mark);
                reported.insert(mark);
            }
        }
        else
            marks.insert(mark, r);
    }
    for (auto i = errors.begin(); i != errors.end(); )
        if (i.value().first == 0)
            i = errors.erase(i);
        else
            ++i;
    for(auto i = unresolved.begin(); i != unresolved.end(); i++)
        if (!marks.contains(i.key()))
            for(int r: i.value())
                errors.insert(r, qMakePair(0, QString("Неразрешенная ссылка: %1").arg(i.key())));
}

void Simulator::update_breakpoints()
{
    breakpoints.fill(0, p_tbprog->rowCount());
    for (int r = 0; r < p_tbprog->rowCount(); r++)
    {
        auto item = p_tbprog->item(r, 0);
        if (item && item->checkState() == Qt::Checked)
            breakpoints[r] = true;
    }
}

bool Simulator::warn_unsaved()
{
    if (saved)
        return true;
    QMessageBox::StandardButton answer = QMessageBox::warning(this,
                                                              "Файл не сохранен!",
                                                              "Сохранить измения?",
                                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                                              QMessageBox::Save);
    if (answer == QMessageBox::Save)
    {
        save();
        return true;
    }
    if (answer == QMessageBox::Discard)
        return true;
    return false;
}

void Simulator::save(QString filename)
{
    QFile f(filename);
    if (f.open(QFile::WriteOnly))
    {
        QTextStream out(&f);
        for (int r = 0; r < p_tbprog->rowCount(); r++)
        {
            auto mark = p_tbprog->item(r, 1);
            auto code = p_tbprog->item(r, 2);
            if (!mark->text().isEmpty())
                out << '@' << mark->text() << ' ';
            out << code->text() << '\n';
        }
        for (auto it = registers.begin(); it != registers.end(); it++)
            out << *it << ' ';
        out << '\n';
        for (auto it = memory.begin(); it != memory.end(); it++)
            out << *it << ' ';
        out << '\n';
        f.close();
    }
}

void Simulator::init()
{
    state = Calculating;
    setState(Editing);
    memory.fill(0, 128);
    registers.fill(0, Command::reg_names.size());
    breakpoints.fill(0, 128);
    errors.clear();
    marks.clear();
    unresolved.clear();
    mark_errors.clear();

    disconnect(p_tbprog,
               SIGNAL(cellChanged(int, int)),
               this,
               SLOT(program_edited(int, int)));

    p_tbprog->clearContents();

    update_memory();
    update_errors();
    update_registers();
    fix_prog_table();

    saved = true;

    update_title();

    connect(p_tbprog,
            SIGNAL(cellChanged(int, int)),
            this,
            SLOT(program_edited(int, int)));

}

void Simulator::setState(Simulator::State s)
{
    if (state == s)
        return;
    state = s;
    for(auto button : {p_btnew, p_btopen, p_btsave, p_btsaveas, p_btdebug, p_btnext, p_btstep, p_btcopy, p_btpaste, p_btinsert, p_btdeleteline, p_btinitmem})
        button->setEnabled(true);
    if (state == Editing)
    {
        connect(p_tbprog,
                SIGNAL(itemSelectionChanged()),
                this,
                SLOT(lock_editing_buttons()));
        update_editing_field();
        lock_editing_buttons();
        p_btnext->setEnabled(false);
        p_btstep->setEnabled(false);
        p_btdebug->setText("Начать выполнение");
    }
    else
    {
        update_editing_field();
        disconnect(p_tbprog,
                SIGNAL(itemSelectionChanged()),
                this,
                SLOT(lock_editing_buttons()));
        p_btdebug->setText("Прекратить выполнение");
        p_btinitmem->setEnabled(false);
        if (state == Calculating)
        {
            p_btnext->setEnabled(false);
            p_btstep->setEnabled(false);
        }
        else
        {
            p_tbprog->clearSelection();
            p_tbprog->selectRow(PC());
        }
    }
    mark_line();
}

int Simulator::execute_one(bool f)
{
    int pc = Command::reg_names.indexOf("PC");
    QString command = p_tbprog->item(registers.at(pc), 2)->text();
    registers[pc]++;
    if (registers[pc] == 128)
        registers[pc] = 0;
    if (command.isEmpty())
        return 1;
    substitute_mark(command);
    if (command.contains("HLT"))
    {
        return 2;
    }
    try {
        cset.exec(command, memory, registers);
    } catch (QString mes) {
        if (f)
            QMessageBox::critical(this,
                              "Ошибка исполнения!",
                              mes);
        return 0;
    }
    catch(...) {
        if (f)
            QMessageBox::critical(this,
                              "Ошибка исполнения!",
                              QString("Команда не может быть выполнена: %1").arg(command));
        return 0;
    }
    return 1;
}

int Simulator::PC()
{
    return registers.at(Command::reg_names.indexOf("PC"));
}

void Simulator::mark_line()
{
    marking_line = true;
    QBrush b(Qt::white);
    for (int r = 0; r < p_tbprog->rowCount(); r++)
        for (int c = 0; c < p_tbprog->columnCount(); c++)
            p_tbprog->item(r, c)->setBackground(b);
    if (state == Breakpoint)
    {
        int r = PC();
        b.setColor(Qt::blue);
        for (int c = 0; c < p_tbprog->columnCount(); c++)
            p_tbprog->item(r, c)->setBackground(b);
    }
    marking_line = false;
}

void Simulator::fix_prog_table()
{
    p_tbprog->setRowCount(128);
    QStringList tbproglabels;
    for (int i = 0; i < p_tbprog->rowCount(); i++)
        tbproglabels << "0x" + QString::number(i, 16);
    p_tbprog->setVerticalHeaderLabels(tbproglabels);
    update_editing_field();
    update_marks();
    update_breakpoints();
}

void Simulator::substitute_mark(QString &command)
{
    if (command.contains('@'))
    {
        int begin = command.indexOf('@');
        int end = command.indexOf(' ', begin);
        QString mark = command.mid(begin + 1, end);
        qDebug() << mark << "replaced with" << marks.value(mark);
        command.replace('@' + mark, QString::number(marks.value(mark)));
    }
}

Simulator::Simulator(QWidget *parent)
    : QWidget(parent),
      filename("Новый файл"),
      saved(true),
      state(Editing),
      memory(128),
      registers(Command::reg_names.size()),
      cset(),
      breakpoints(128),
      errors(),
      marks(),
      unresolved(),
      marking_line(false)
{
    p_ltmain = new QVBoxLayout(this);

    p_ltfile = new QHBoxLayout;
    p_btnew = new QPushButton("Новый файл");
    p_btopen = new QPushButton("Открыть файл");
    p_btsave = new QPushButton("Сохранить файл");
    p_btsaveas = new QPushButton("Сохранить файл как...");
    p_ltfile->addWidget(p_btnew);
    p_ltfile->addWidget(p_btopen);
    p_ltfile->addWidget(p_btsave);
    p_ltfile->addWidget(p_btsaveas);
    connect(p_btnew,
            SIGNAL(clicked()),
            this,
            SLOT(file_button()));
    connect(p_btopen,
            SIGNAL(clicked()),
            this,
            SLOT(file_button()));
    connect(p_btsave,
            SIGNAL(clicked()),
            this,
            SLOT(save()));
    connect(p_btsaveas,
            SIGNAL(clicked()),
            this,
            SLOT(saveas()));

    p_ltbtdebug = new QHBoxLayout;
    p_btdebug = new QPushButton("Начать выполнение");
    p_btnext = new QPushButton("Следующая точка останова");
    p_btstep = new QPushButton("Следующая команда");
    p_ltbtdebug->addWidget(p_btdebug);
    p_ltbtdebug->addWidget(p_btnext);
    p_ltbtdebug->addWidget(p_btstep);

    connect(p_btdebug,
            SIGNAL(clicked()),
            this,
            SLOT(execute()));
    connect(p_btnext,
            SIGNAL(clicked()),
            this,
            SLOT(execute()));
    connect(p_btstep,
            SIGNAL(clicked()),
            this,
            SLOT(execute()));

    p_bttest = new QPushButton("Тестировать сортировки");
    p_gencode = new QPushButton("Сгенерировать код");

    QHBoxLayout *lt = new QHBoxLayout;
    lt->addWidget(p_bttest);
    lt->addWidget(p_gencode);

    connect(p_bttest,
            SIGNAL(clicked()),
            this,
            SLOT(test()));

    connect(p_gencode,
            SIGNAL(clicked()),
            this,
            SLOT(gen_code()));

    //TODO: коннекты
    //TODO: сделать кнопки неактивными

    p_ltbtedit = new QHBoxLayout;
    p_btcopy = new QPushButton("Копировать");
    p_btpaste = new QPushButton("Вставить");
    p_btinsert = new QPushButton("Вставить строку");
    p_btdeleteline = new QPushButton("Удалить строчку");
    p_btinitmem = new QPushButton("Инициализировать память и регистры");
    p_ltbtedit->addWidget(p_btcopy);
    p_ltbtedit->addWidget(p_btpaste);
    p_ltbtedit->addWidget(p_btinsert);
    p_ltbtedit->addWidget(p_btdeleteline);
    p_ltbtedit->addWidget(p_btinitmem);


    connect(p_btinitmem,
            SIGNAL(clicked()),
            this,
            SLOT(init_mem()));

    for (auto button: {p_btcopy, p_btpaste, p_btinsert, p_btdeleteline})
        connect(button,
            SIGNAL(clicked()),
            this,
            SLOT(edit_buttons()));
    //TODO: коннекты
    //TODO: сделать кнопки неактивными

    qDebug("buttons");

    //p_spreg = new QSplitter(Qt::Horizontal);
    //p_spreg->setChildrenCollapsible(false);
    //p_spreg->setOpaqueResize(true);

    p_tbreg = new QTableWidget(registers.size(), 1);
    p_tbreg->setWindowTitle("Регистры");
    p_tbreg->setWindowFlag(Qt::SubWindow);
    p_tbreg->setMinimumSize(50, 515);
    p_tbreg->setMaximumSize(93, 530);
    p_tbreg->show();
    p_tbreg->move(50, 100);
    //p_spreg->addWidget(p_tbreg);
    p_tbreg->setHorizontalHeaderLabels({"Val"});
    p_tbreg->setVerticalHeaderLabels(Command::reg_names);

    p_sperr = new QSplitter(Qt::Vertical);
    p_tbprog = new QTableWidget(128, 3);
    p_tbprog->setHorizontalHeaderLabels({"ТО", "Метка", "Код"});
    p_tbprog->setColumnWidth(0, 20);
    p_tbprog->setColumnWidth(1, 80);
    p_tbprog->setColumnWidth(2, 340);

    p_tberr = new QTableWidget(1, 3);
    p_tberr->setHorizontalHeaderLabels({"Тип", "Адрес", "Сообщение"});
    p_tberr->setColumnWidth(0, 50);
    p_tberr->setColumnWidth(1, 50);
    p_tberr->setColumnWidth(2, 420);
    p_sperr->addWidget(p_tbprog);
    p_sperr->addWidget(p_tberr);

    p_tbmem = new QTableWidget(16, 8);
    p_tbmem->setWindowTitle("Память");
    p_tbmem->setWindowFlag(Qt::SubWindow);
    p_tbmem->setMinimumSize(440, 625);
    //p_tbmem->setMaximumSize(250, 750);
    p_tbmem->show();
    p_tbmem->move(150, 100);
    QStringList tbmemlb;
    for(int i = 0; i < p_tbmem->rowCount(); i++)
        tbmemlb << "0x" + QString::number(i * p_tbmem->columnCount(), 16);
    p_tbmem->setVerticalHeaderLabels(tbmemlb);
    tbmemlb.clear();
    for (int i = 0; i < p_tbmem->columnCount(); i++)
        tbmemlb << QString::number(i);
    p_tbmem->setHorizontalHeaderLabels(tbmemlb);

    //p_spreg->addWidget(p_tbmem);

    qDebug() << "splitter";
    qDebug() << p_ltmain << p_ltfile << p_ltbtdebug << p_ltbtedit;// << p_spreg;

    //TODO: настройки таблиц, данные

    p_ltmain->addLayout(p_ltfile);
    p_ltmain->addLayout(p_ltbtdebug);
    p_ltmain->addLayout(p_ltbtedit);
    p_ltmain->addLayout(lt);
    //p_ltmain->addWidget(p_spreg);
    p_ltmain->addWidget(p_sperr);


    init();

    /*connect(p_tbreg,
            SIGNAL(cellChanged(int, int)),
            this,
            SLOT(regtb_change(int, int)));*/

    qDebug("constructed");


}

Simulator::~Simulator()
{
    delete p_tbreg;
    delete p_tbmem;
}

