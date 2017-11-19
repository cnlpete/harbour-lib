/*
 * Copyright (C) 2017 Jolla Ltd.
 * Copyright (C) 2017 Slava Monich <slava.monich@jolla.com>
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
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HarbourPluginLoader.h"
#include "HarbourDebug.h"

#include <qqml.h>
#include <QQmlEngine>
#include <QQmlTypesExtensionInterface>

#include <QStringList>
#include <QPluginLoader>
#include <QTextStream>
#include <QFile>

// This hack allows (in some cases) to use prohibited QML imports by
// re-registering them under a different name

// PRIVATE QT API!
class Q_QML_EXPORT QQmlType
{
public:
    int typeId() const;
    int qListTypeId() const;
    typedef void (*CreateFunc)(void *);
    CreateFunc createFunction() const;
    int createSize() const;
    const QMetaObject *metaObject() const;
    int parserStatusCast() const;
    int propertyValueSourceCast() const;
    int propertyValueInterceptorCast() const;
};

// PRIVATE QT API!
class Q_QML_EXPORT QQmlMetaType
{
public:
    static QQmlType* qmlType(const QString &qualifiedName, int, int);
};

// ==========================================================================
// HarbourPluginLoader::Private
// ==========================================================================

class HarbourPluginLoader::Private {
public:
    Private(QQmlEngine* aEngine, QString aModule, int aMajor, int aMinor);
    ~Private();

    QQmlType* qmlType(QString aName);

    void reRegisterType(QQmlType* aType, const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aOriginalQmlName, const char* aNewQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);

public:
    bool iTypesRegistered;
    QPluginLoader* iPlugin;
    QString iModule;
    int iMajor;
    int iMinor;
};

HarbourPluginLoader::Private::Private(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor) :
    iTypesRegistered(false),
    iPlugin(pluginLoader(aEngine, aModule)),
    iModule(aModule),
    iMajor(aMajor),
    iMinor(aMinor)
{}

HarbourPluginLoader::Private::~Private()
{
    delete iPlugin;
}

QQmlType*
HarbourPluginLoader::Private::qmlType(
    QString aName)
{
    if (iPlugin) {
        if (!iTypesRegistered) {
            iTypesRegistered = true;
            QObject* root = iPlugin->instance();
            if (root) {
                QQmlTypesExtensionInterface* ext =
                    qobject_cast<QQmlTypesExtensionInterface*>(root);
                if (ext) {
                    QByteArray str = iModule.toLocal8Bit();
                    ext->registerTypes(str.constData());
                }
            } else {
                HWARN("Could not load" << qPrintable(iPlugin->fileName()));
            }
        }
        QString fullName(iModule + '/' + aName);
        QQmlType* type = QQmlMetaType::qmlType(fullName, iMajor, iMinor);
        if (!type) {
            HWARN("Failed to load" << fullName);
        }
        return type;
    }
    return NULL;
}

void
HarbourPluginLoader::Private::reRegisterType(
    const char* aQmlName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with the same type name (in different module)
    reRegisterType(qmlType(aQmlName), aQmlName, aModule, aMajor, aMinor);
}

void
HarbourPluginLoader::Private::reRegisterType(
    const char* aOrigName,
    const char* aNewName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with a different type name (and a module)
    reRegisterType(qmlType(aOrigName), aNewName, aModule, aMajor, aMinor);
}

// Re-registers the existing QML type under a different name/module
void
HarbourPluginLoader::Private::reRegisterType(
    QQmlType* aType,
    const char* aQmlName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    if (aType) {
        QQmlPrivate::RegisterType type = {
            0, // int version;
            aType->typeId(),  // int typeId;
            aType->qListTypeId(), // int listId;
            aType->createSize(), // int objectSize;
            aType->createFunction(), // void (*create)(void *);
            QString(), // QString noCreationReason;
            aModule, // const char *uri;
            aMajor, // int versionMajor;
            aMinor, // int versionMinor;
            aQmlName, // const char *elementName;
            aType->metaObject(), // const QMetaObject *metaObject;
#if 0 // We don't need those, it seems
            aType->attachedPropertiesFunction(),
            aType->attachedPropertiesType(),
#else
            Q_NULLPTR, // QQmlAttachedPropertiesFunc attachedPropertiesFunction;
            Q_NULLPTR, // const QMetaObject *attachedPropertiesMetaObject;
#endif
            aType->parserStatusCast(), // int parserStatusCast;
            aType->propertyValueSourceCast(), // int valueSourceCast;
            aType->propertyValueInterceptorCast(), // int valueInterceptorCast;
            Q_NULLPTR, // QObject *(*extensionObjectCreate)(QObject *);
            Q_NULLPTR, // const QMetaObject *extensionMetaObject;
            Q_NULLPTR, // QQmlCustomParser *customParser;
            0  // int revision;
        };
        QQmlPrivate::qmlregister(QQmlPrivate::TypeRegistration, &type);
    }
}

// ==========================================================================
// HarbourPluginLoader
// ==========================================================================

HarbourPluginLoader::HarbourPluginLoader(
    QQmlEngine* aEngine,
    QString aModule,
    int aMajor,
    int aMinor) :
    iPrivate(new Private(aEngine, aModule, aMajor, aMinor))
{}

HarbourPluginLoader::~HarbourPluginLoader()
{
    delete iPrivate;
}

QPluginLoader*
HarbourPluginLoader::pluginLoader(
    QQmlEngine* aEngine,
    QString aModule)
{
    QStringList pathList = aEngine->importPathList();
    aModule.replace('.', '/');
    const int n = pathList.count();
    for (int i=0; i<n; i++) {
        QString dir(pathList.at(i));
        QPluginLoader* loader = pluginLoader(dir.append('/').append(aModule));
        if (loader) {
            if (loader->load()) {
                HDEBUG("loaded" << qPrintable(loader->fileName()));
                return loader;
            } else {
                HWARN("Failed to load" << qPrintable(loader->fileName()));
                delete loader;
            }
        }
    }
    return NULL;
}

QPluginLoader*
HarbourPluginLoader::pluginLoader(
    QString aPluginDir)
{
    QString qmldir(QString(aPluginDir).append('/').append("qmldir"));
    QFile f(qmldir);
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            static const QString plugin("plugin");
            QString line = in.readLine();
            if (line.indexOf(plugin) >= 0) {
                QStringList parts = line.split(' ', QString::SkipEmptyParts);
                if (parts.count() == 2 && parts.at(0) == plugin) {
                    QString path(QString(aPluginDir).append("/lib").
                        append(parts.at(1)).append(".so"));
                    if (QFile::exists(path)) {
                        return new QPluginLoader(path);
                    }
                }
            }
        }
    }
    return NULL;
}

void
HarbourPluginLoader::reRegisterType(
    const char* aQmlName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with the same type name (in different module)
    iPrivate->reRegisterType(aQmlName, aModule, aMajor, aMinor);
}

void HarbourPluginLoader::reRegisterType(
    const char* aOrigName,
    const char* aNewName,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    // Re-register with a different type name (and a module)
    iPrivate->reRegisterType(aOrigName, aNewName, aModule, aMajor, aMinor);
}

bool
HarbourPluginLoader::isValid() const
{
    return iPrivate->iPlugin != NULL;
}
