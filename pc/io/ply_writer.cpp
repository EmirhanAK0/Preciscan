#include "ply_writer.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

namespace io
{

bool writePLY(const QString& path, const QVector<QVector3D>& points)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "PLY dosyasi acilamadi:" << path;
        return false;
    }

    QTextStream out(&file);

    // Header
    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "element vertex " << points.size() << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "end_header\n";

    // Data
    for (const auto& p : points)
    {
        out << p.x() << " " << p.y() << " " << p.z() << "\n";
    }

    file.close();
    return true;
}


bool readPLY(const QString& path, QVector<QVector3D>& points)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString format = "ascii";
    int vertexCount = 0;
    bool headerEnded = false;
    int vertexSize = 0;
    int xOffset = -1, yOffset = -1, zOffset = -1;
    bool inVertexElement = false;

    // Read header (ASCII)
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QString strLine(line);
        if (strLine.startsWith("format ")) {
            format = strLine.split(" ", Qt::SkipEmptyParts).value(1);
        } else if (strLine.startsWith("element ")) {
            QStringList parts = strLine.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                if (parts[1] == "vertex") {
                    vertexCount = parts[2].toInt();
                    inVertexElement = true;
                } else {
                    inVertexElement = false;
                }
            }
        } else if (strLine.startsWith("property ") && inVertexElement) {
            QStringList parts = strLine.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString type = parts[1];
                QString name = parts[2];
                int typeSize = 0;
                if (type == "float" || type == "float32" || type == "int" || type == "int32" || type == "uint" || type == "uint32") typeSize = 4;
                else if (type == "double" || type == "float64") typeSize = 8;
                else if (type == "short" || type == "int16" || type == "ushort" || type == "uint16") typeSize = 2;
                else if (type == "char" || type == "int8" || type == "uchar" || type == "uint8") typeSize = 1;

                if (name == "x") xOffset = vertexSize;
                else if (name == "y") yOffset = vertexSize;
                else if (name == "z") zOffset = vertexSize;

                vertexSize += typeSize;
            }
        } else if (strLine == "end_header") {
            headerEnded = true;
            break;
        }
    }

    if (!headerEnded || vertexCount == 0 || xOffset == -1 || yOffset == -1 || zOffset == -1)
        return false;

    points.reserve(points.size() + vertexCount);

    if (format == "ascii") {
        QTextStream in(&file);
        for (int i = 0; i < vertexCount && !in.atEnd(); ++i) {
            QString line = in.readLine().trimmed();
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                points.push_back(QVector3D(parts[0].toFloat(), parts[1].toFloat(), parts[2].toFloat()));
            }
        }
    } else if (format == "binary_little_endian") {
        for (int i = 0; i < vertexCount; ++i) {
            if (file.atEnd()) break;
            QByteArray vertexData = file.read(vertexSize);
            if (vertexData.size() < vertexSize) break;
            
            float x, y, z;
            memcpy(&x, vertexData.constData() + xOffset, sizeof(float));
            memcpy(&y, vertexData.constData() + yOffset, sizeof(float));
            memcpy(&z, vertexData.constData() + zOffset, sizeof(float));
            points.push_back(QVector3D(x, y, z));
        }
    } else {
        // unsupported format
        return false;
    }

    return true;
}

} // namespace io
