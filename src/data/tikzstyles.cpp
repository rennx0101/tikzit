/*
    TikZiT - a GUI diagram editor for TikZ
    Copyright (C) 2018 Aleks Kissinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tikzstyles.h"
#include "tikzassembler.h"

#include <QDebug>
#include <QColorDialog>
#include <QFile>
#include <QFileInfo>

TikzStyles::TikzStyles(QObject *parent) : QObject(parent)
{
    _nodeStyles = new NodeStyleList(this);
}

Style *TikzStyles::nodeStyle(QString name) const
{
    Style *s = _nodeStyles->style(name);

    return (s == nullptr) ? unknownStyle : s;
}

Style *TikzStyles::edgeStyle(QString name) const
{
    foreach (Style *s , _edgeStyles)
        if (s->name() == name) return s;
    return noneEdgeStyle;
}


void TikzStyles::clear()
{
    _nodeStyles->clear();
    _edgeStyles.clear();
}

bool TikzStyles::loadStyles(QString fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString styleTikz = in.readAll();
        file.close();

        clear();
        TikzAssembler ass(this);
        return ass.parse(styleTikz);
    } else {
        return false;
    }
}

bool TikzStyles::saveStyles(QString fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << tikz();
        file.close();
        return true;
    }
    return false;
}

void TikzStyles::refreshModels(QStandardItemModel *nodeModel, QStandardItemModel *edgeModel, QString category, bool includeNone)
{
    nodeModel->clear();
    edgeModel->clear();


    //QString f = tikzit->styleFile();
    //ui->styleFile->setText(f);

    QStandardItem *it;

    if (includeNone) {
        it = new QStandardItem(noneStyle->icon(), noneStyle->name());
        it->setEditable(false);
        it->setData(noneStyle->name());
        nodeModel->appendRow(it);
        it->setTextAlignment(Qt::AlignCenter);
        it->setSizeHint(QSize(48,48));
    }

    Style *ns;
    for (int i = 0; i < _nodeStyles->length(); ++i) {
        ns = _nodeStyles->style(i);
        if (category == "" || category == ns->propertyWithDefault("tikzit category", "", false))
        {
            it = new QStandardItem(ns->icon(), ns->name());
            it->setEditable(false);
            it->setData(ns->name());
            it->setSizeHint(QSize(48,48));
            nodeModel->appendRow(it);
        }
    }

    if (includeNone) {
        it = new QStandardItem(noneEdgeStyle->icon(), noneEdgeStyle->name());
        it->setEditable(false);
        it->setData(noneEdgeStyle->name());
        edgeModel->appendRow(it);
    }

    foreach(Style *es, _edgeStyles) {
        //if (category == "" || category == es->propertyWithDefault("tikzit category", "", false))
        //{
            it = new QStandardItem(es->icon(), es->name());
            it->setEditable(false);
            it->setData(es->name());
            edgeModel->appendRow(it);
        //}
    }
}

QStringList TikzStyles::categories() const
{
    QMap<QString,bool> cats; // use a QMap to keep keys sorted
    cats.insert("", true);
    Style *ns;
    for (int i = 0; i < _nodeStyles->length(); ++i) {
        ns = _nodeStyles->style(i);
        cats.insert(ns->propertyWithDefault("tikzit category", "", false), true);
    }
    //foreach (EdgeStyle *s, _edgeStyles) cats << s->propertyWithDefault("tikzit category", "", false);
    return QStringList(cats.keys());
}

QString TikzStyles::tikz() const
{
    QString str;
    QTextStream code(&str);

    code << "% TiKZ style file generated by TikZiT. You may edit this file manually,\n";
    code << "% but some things (e.g. comments) may be overwritten. To be readable in\n";
    code << "% TikZiT, the only non-comment lines must be of the form:\n";
    code << "% \\tikzstyle{NAME}=[PROPERTY LIST]\n\n";

    code << "% Node styles\n";
    code << _nodeStyles->tikz();

    code << "\n% Edge styles\n";
    foreach (Style *s, _edgeStyles) code << s->tikz() << "\n";

    code.flush();
    return str;
}

void TikzStyles::addStyle(QString name, GraphElementData *data)
{
    Style *s = new Style(name, data);
    if (s->isEdgeStyle())
    { // edge style
        _edgeStyles << s;
    } else { // node style
        _nodeStyles->addStyle(new Style(name, data));
    }
}


