/*
 * Copyright (C) 2020-2021 Jolla Ltd.
 * Copyright (C) 2020-2021 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#ifndef HARBOUR_SYSTEM_INFO_H
#define HARBOUR_SYSTEM_INFO_H

#include <QObject>
#include <QVector>

class QQmlEngine;
class QJSEngine;

class HarbourSystemInfo: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HarbourSystemInfo)
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(QString osVersion READ osVersion CONSTANT)

public:
    explicit HarbourSystemInfo(QObject* aParent = NULL);
    ~HarbourSystemInfo();

    // Callback for qmlRegisterSingletonType<HarbourSystemInfo>
    static QObject* createSingleton(QQmlEngine* aEngine, QJSEngine* aScript);

    QString osName() const;
    QString osVersion() const;

    Q_INVOKABLE QString packageVersion(QString aPackage);
    Q_INVOKABLE int osVersionCompare(QString aVersion);
    Q_INVOKABLE static int compareVersions(QString aVersion1, QString aVersion2);

    static QString queryPackageVersion(QString aVersion);
    static int osVersionCompareWith(QString aVersion);

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_SYSTEM_INFO_H
