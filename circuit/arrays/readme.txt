Здесь располагаются массивы для тестов. Их необходимо загрузить в память для операндов.

Подготовка тестовых массивов:
(устройство памяти описано в отчете в разделе 3.2.2)
Массив для сортировки располагается по адресам 0x00-0x4f. Младшая часть
широкого операнда имеет младший адрес. По адресу 0x70 должно быть 
записан размер массива. ВНИМАНИЕ! Запись по адресу 0x70 значения больше
40 приведет к ошибке, аппарано или программно это не проверяется.
