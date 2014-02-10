Author-Detection-Trigram
========================

Authorship detection by Alexandr Zhelubenkov, supervised by Dr Mstislav Maslennikov

Nickname: anAuthor 13c

This is a project for the "Mathematical methods for text analysis" course in spring 2013 in MSU. 

Kindly refer to 
- Report.pdf
- vk.com/mathtexts -> achievements of listeners (http://vk.com/topic-50553132_28815796)

Compulsory citation:
Alexandr Zhelubenkov and Mstislav Maslennikov, 2013. "Author detection using Support Vector Machines". In "Mathematical methods for text analysis" speckours, vk.com/mathtexts.


Instructions
============

Формат директорий для тестирования и классификации следующий:
В директории для тестирования должны лежать поддиректории, соответствующие авторам. Имя автора определяется именем поддиректории. В каждой поддиректории должны лежать текстовые файлы - тексты для тестирования/классификации.

Формат файла с предсказанными результатами и истиными результатами слежующий:
text1 auth_name1
text2 auth_name2
...
textN auth_nameN

Исходная тренировочная выборка 
http://modis.ispras.ru/tpc/wp-content/uploads/2011/10/trainingSet.zip

Исходная тестовая выборка
http://modis.ispras.ru/tpc/wp-content/uploads/2011/12/testingSet.zip

Файл answer.txt содержит истинные результат для исходной тестовой выборки

Файл model.txt содердит модель, построенную на тренировочной выборке с количеством триграмм, равным 500.

Авторы, на которых тренировалась и тестировалась модель:
amfiteatrov
best-marl
bunin
chekhov
dostoevski
gogol
grigorovich
herzen
leontjev
lermontov
leskov
melnikov-p
panaev
pisemski
pushkin
reshetnikov
saltykov
sollogub
tolstoy-l
turgenev
