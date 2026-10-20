/*****************************************************************************
 * help.cpp : Help and About dialogs
 ****************************************************************************
 * Copyright (C) 2007 the VideoLAN team
 *
 * Authors: Jean-Baptiste Kempf <jb (at) videolan.org>
 *          Rémi Duraffort <ivoire (at) via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "qt.hpp"
#include "help.hpp"
#include "aboutmodel.hpp"
#include "maininterface/mainctx.hpp"
#include "util/color_scheme_model.hpp"
#include "util/qt_dirs.hpp"
#include "util/interface_translator.hpp"


#include <vlc_intf_strings.h>

#ifdef UPDATE_CHECK
# include "util/github_release_service.hpp"
# include <QDesktopServices>
#endif

#include <QTextBrowser>
#include <QString>
#include <QDialogButtonBox>
#include <QEvent>
#include <QDate>
#include <QPushButton>
#include <QRegularExpression>
#include <QApplication>
#include <QStyleHints>
#include <QScrollBar>

#include <cassert>

HelpDialog::HelpDialog( qt_intf_t *_p_intf ) : QVLCFrame( _p_intf )

{
    setWindowTitle( qtr( "Help" ) );
    setWindowRole( "vlc-help" );
    setMinimumSize( 350, 300 );

    QVBoxLayout *layout = new QVBoxLayout( this );

    QTextBrowser *helpBrowser = new QTextBrowser( this );
    helpBrowser->setOpenExternalLinks( true );
    helpBrowser->setHtml( qfut(I_LONGHELP) );

    QDialogButtonBox *closeButtonBox = new QDialogButtonBox( this );
    auto* closeButton = new QPushButton(qtr("&Close"));
    closeButtonBox->addButton(closeButton, QDialogButtonBox::RejectRole);
    closeButtonBox->setFocus();

    layout->addWidget( helpBrowser );
    layout->addWidget( closeButtonBox );

    connect( closeButtonBox, &QDialogButtonBox::rejected, this, &HelpDialog::close );
    if (auto* translator = InterfaceTranslator::instance())
        connect(translator, &InterfaceTranslator::languageChanged, this, [this, helpBrowser, closeButton] {
            const auto scroll = helpBrowser->verticalScrollBar()->value();
            setWindowTitle(qtr("Help"));
            helpBrowser->setHtml(qfut(I_LONGHELP));
            helpBrowser->verticalScrollBar()->setValue(scroll);
            closeButton->setText(qtr("&Close"));
        });
    restoreWidgetPosition( "Help", QSize( 500, 450 ) );
}

HelpDialog::~HelpDialog()
{
    saveWidgetPosition( "Help" );
}

AboutDialog::AboutDialog( qt_intf_t *_p_intf)
            : QVLCDialog( nullptr, _p_intf ), b_advanced( false )
{
    /* Build UI */
    ui.setupUi( this );
    setWindowTitle( qtr( "About" ) );
    setWindowRole( "vlc-about" );
    setWindowModality( Qt::WindowModal );

    ui.version->setText(qfu( " " VERSION_MESSAGE ) );
    const auto updateText = [this] {
        setWindowTitle(qtr("About"));
        ui.title->setText("<html><head/><body><p><span style=\" font-size:26pt;\"> " + qtr( "VLC media player" ) + " </span></p></body></html>");

        ui.MainBlabla->setText("<html><head/><body>" +
        qtr( "<p>VLC media player is a free and open source media player, encoder, and streamer made by the volunteers of the <a href=\"https://www.videolan.org/\"><span style=\" text-decoration: underline; color:#0057ae;\">VideoLAN</span></a> community.</p><p>VLC uses its internal codecs, works on essentially every popular platform, and can read almost all files, CDs, DVDs, network streams, capture cards and other media formats!</p><p><a href=\"https://www.videolan.org/contribute/\"><span style=\" text-decoration: underline; color:#0057ae;\">Help and join us!</span></a>" ).remove(QStringLiteral("color:#0057ae;")) +
        "</p>" + AboutModel::getDesignAttribution() + "</body> </html>");
        ui.licenseButton->setText(qtr("License"));
        ui.authorsButton->setText(qtr("Authors"));
        ui.creditsButton->setText(qtr("Credits"));
        ui.UpdateLabel->setText(qtr("Update"));
        ui.checkUpdates->setText(qtr("Update"));
    };
    updateText();

    const QDate today = QDate::currentDate();
    if( today.month() == 4 && today.day() == 1
            && var_InheritBool( p_intf, "qt-icon-change" ) )
        ui.VLCcone->setPixmap( QPixmap( ":/logo/vlc128-aprilfools.png" ) );

    ui.update->hide();

    /* GPL License */
    ui.licensePage->setText( AboutModel::getLicense() );

    /* People who helped */
    ui.creditPage->setText( qfu( psz_thanks ) );

    /* People who wrote the software */
    ui.authorsPage->setText( qfu( psz_authors ) );

    connect(ui.licenseButton, &QPushButton::clicked, this, &AboutDialog::showLicense);

    connect(ui.authorsButton, &QPushButton::clicked, this, &AboutDialog::showAuthors);

    connect(ui.creditsButton, &QPushButton::clicked, this, &AboutDialog::showCredit);

    const auto updateLinkColor = [this] {
        const QColor color = palette().color(QPalette::Link);
        QString html = ui.MainBlabla->text();
        html.remove(QRegularExpression(QStringLiteral(" style=\"color:#[0-9a-fA-F]{6};\"")));
        html.replace(QStringLiteral("<a "), QStringLiteral("<a style=\"color:%1;\" ").arg(color.name()));
        ui.MainBlabla->setText(html);
    };
    updateLinkColor();
    if (auto* translator = InterfaceTranslator::instance())
        connect(translator, &InterfaceTranslator::languageChanged, this, [updateText, updateLinkColor] {
            updateText();
            updateLinkColor();
        });
    if (p_intf->p_mi)
    {
        connect(p_intf->p_mi, &MainCtx::redesignAccentChanged, this, updateLinkColor);
        connect(p_intf->p_mi->getColorScheme(), &ColorSchemeModel::currentChanged, this, updateLinkColor);
        connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, updateLinkColor);
    }

    ui.version->installEventFilter( this );
}

void AboutDialog::showLicense()
{
    ui.stackedWidget->setCurrentWidget( ui.licensePage );
}

void AboutDialog::showAuthors()
{
    ui.stackedWidget->setCurrentWidget( ui.authorsPage );
}

void AboutDialog::showCredit()
{
    ui.stackedWidget->setCurrentWidget( ui.creditPage );
}

bool AboutDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress )
    {
        if( obj == ui.version )
        {
            if( !b_advanced )
            {
                ui.version->setText(qfu( VLC_CompileBy() )+ "@" + qfu( VLC_CompileHost() )
                    + " " + __DATE__ + " " + __TIME__);
                b_advanced = true;
            }
            else
            {
                ui.version->setText(qfu( " " VERSION_MESSAGE ) );
                b_advanced = false;
            }
            return true;
        }
        return false;
    }

    return QVLCDialog::eventFilter( obj, event);
}

void AboutDialog::showEvent( QShowEvent *event )
{
    ui.stackedWidget->setCurrentWidget( ui.blablaPage );
    QVLCDialog::showEvent( event );
}

#ifdef UPDATE_CHECK

class UpdateModelPrivate
{
public:
    GithubReleaseService* service = nullptr;
    std::optional<vlc::updates::Release> release;
    UpdateModel::Status status = UpdateModel::Unchecked;
    bool explicitCheck = false;
};

UpdateModel::UpdateModel(qt_intf_t * p_intf)
    : d_ptr(new UpdateModelPrivate)
{
    Q_UNUSED(p_intf);
    Q_D(UpdateModel);
    d->service = new GithubReleaseService(this);
    connect(d->service, &GithubReleaseService::finished, this, [this](bool success) {
        Q_D(UpdateModel);
        d->release = d->service->release();
        d->status = !success ? CheckFailed : d->release ? NeedUpdate : UpToDate;
        emit updateStatusChanged();
    });
}

UpdateModel::~UpdateModel() = default;

void UpdateModel::checkUpdate(bool explicitCheck)
{
    Q_D(UpdateModel);
    // A manual request during an automatic check should still show its result.
    if (explicitCheck && !d->explicitCheck)
    {
        d->explicitCheck = true;
        emit explicitCheckChanged();
    }
    if (d->status == Checking)
        return;
    d->release.reset();
    const auto installed = vlc::updates::Version::parse(QStringLiteral(PACKAGE_VERSION));
    d->status = installed ? Checking : CheckFailed;
    emit updateStatusChanged();
    if (installed)
        d->service->check(*installed, vlc::updates::Target::current());
}

bool UpdateModel::openReleasePage()
{
    Q_D(UpdateModel);
    return d->status == NeedUpdate && d->release &&
           QDesktopServices::openUrl(d->release->pageUrl);
}

// Keep the existing invokables for interface compatibility. GitHub discovery
// never hands an unsigned asset to the core's signed download/install backend.
bool UpdateModel::download(QString destDir)
{
    Q_UNUSED(destDir);
    return openReleasePage();
}

bool UpdateModel::download()
{
    return openReleasePage();
}

void UpdateModel::resetStatus()
{
    Q_D(UpdateModel);
    if (d->status == Unchecked || d->status == Checking)
        return;
    d->release.reset();
    d->status = Unchecked;
    emit updateStatusChanged();
}

UpdateModel::Status UpdateModel::updateStatus() const
{
    Q_D(const UpdateModel);
    return d->status;
}

int UpdateModel::getMajor() const
{
    Q_D(const UpdateModel);
    return d->release ? int(d->release->version.major) : 0;
}
int UpdateModel::getMinor() const
{
    Q_D(const UpdateModel);
    return d->release ? int(d->release->version.minor) : 0;
}
int UpdateModel::getRevision() const
{
    Q_D(const UpdateModel);
    return d->release ? int(d->release->version.patch) : 0;
}
int UpdateModel::getExtra() const
{
    return 0;
}
QString UpdateModel::getVersion() const
{
    Q_D(const UpdateModel);
    return d->release ? d->release->version.text : QString();
}
QString UpdateModel::getDescription() const
{
    Q_D(const UpdateModel);
    return d->release ? d->release->notes : QString();
}
QString UpdateModel::getUrl() const
{
    Q_D(const UpdateModel);
    return d->release ? d->release->pageUrl.toString() : QString();
}
QString UpdateModel::getAssetUrl() const
{
    Q_D(const UpdateModel);
    return d->release ? d->release->asset.url.toString() : QString();
}

double UpdateModel::getProgress() const
{
    return 0.0;
}

bool UpdateModel::explicitCheck() const
{
    Q_D(const UpdateModel);
    return d->explicitCheck;
}

void UpdateModel::resetExplicitCheck()
{
    Q_D(UpdateModel);
    if (!d->explicitCheck)
        return;
    d->explicitCheck = false;
    emit explicitCheckChanged();
}

/*****************************************************************************
 * UpdateDialog
 *****************************************************************************/

UpdateDialog::UpdateDialog( qt_intf_t *_p_intf ) : QVLCFrame( _p_intf )
{
    /* build Ui */
    ui.setupUi( this );
    ui.updateDialogButtonBox->addButton( new QPushButton( qtr("&Close"), this ),
                                         QDialogButtonBox::RejectRole );
    QPushButton *recheckButton = new QPushButton( qtr("&Recheck version"), this );
    ui.updateDialogButtonBox->addButton( recheckButton, QDialogButtonBox::ActionRole );

    ui.updateNotifyButtonBox->addButton( new QPushButton( qtr("View release on &GitHub"), this ),
                                         QDialogButtonBox::AcceptRole );
    ui.updateNotifyButtonBox->addButton( new QPushButton( qtr("&Dismiss"), this ),
                                         QDialogButtonBox::RejectRole );

    setWindowTitle( qtr( "VLC media player updates" ) );
    setWindowRole( "vlc-update" );

    BUTTONACT( recheckButton, &UpdateDialog::checkOrDownload );
    connect( ui.updateDialogButtonBox, &QDialogButtonBox::rejected, this, &UpdateDialog::close );

    connect( ui.updateNotifyButtonBox, &QDialogButtonBox::accepted, this, &UpdateDialog::checkOrDownload );
    connect( ui.updateNotifyButtonBox, &QDialogButtonBox::rejected, this, &UpdateDialog::close );

    setMinimumSize( 300, 300 );
    setMaximumSize( 500, 300 );

    restoreWidgetPosition( "Update", maximumSize() );

    m_model = p_intf->p_mi->getUpdateModel();
    connect(m_model, &UpdateModel::updateStatusChanged, this, &UpdateDialog::updateUI);
    /* update status*/
    updateUI();
}

UpdateDialog::~UpdateDialog()
{
    saveWidgetPosition( "Update" );
}

bool UpdateDialog::event(QEvent* event)
{
    assert(event);

    if (Q_LIKELY(m_model))
    {
        switch(event->type())
        {
        case QEvent::Hide:
        case QEvent::Show:
        case QEvent::Close:
        case QEvent::Destroy:
            m_model->resetExplicitCheck();
            break;
        default:
            break;
        }
    }

    return QVLCFrame::event(event);
}

/* Check for updates */
void UpdateDialog::checkOrDownload()
{
    switch (m_model->updateStatus()) {
    case UpdateModel::Unchecked:
    case UpdateModel::UpToDate:
    case UpdateModel::CheckFailed:
    {
        ui.stackedWidget->setCurrentWidget( ui.updateRequestPage );
        m_model->checkUpdate(true);
        break;
    }
    case UpdateModel::NeedUpdate:
    {
        if (m_model->openReleasePage())
            toggleVisible();
        break;
    }
    default: // Checking
        break;
    }
}

/* Show the result of release discovery. */
void UpdateDialog::updateUI( )
{
    switch (m_model->updateStatus()) {
    case UpdateModel::NeedUpdate:
    {
        ui.stackedWidget->setCurrentWidget( ui.updateNotifyPage );
        ui.updateNotifyLabel->setText(qtr("A new version of VLC (%1) is available.")
                                     .arg(m_model->getVersion()));
        ui.updateNotifyTextEdit->setPlainText(m_model->getDescription());
        break;
    }
    case UpdateModel::UpToDate:
    {
        ui.stackedWidget->setCurrentWidget( ui.updateDialogPage );
        ui.updateDialogLabel->setText(
            qtr("No newer compatible release is available for this build.") );
        break;
    }
    case UpdateModel::CheckFailed:
    {
        ui.stackedWidget->setCurrentWidget( ui.updateDialogPage );
        ui.updateDialogLabel->setText(
            qtr( "An error occurred while checking for updates..." ) );
        break;
    }
    case UpdateModel::Checking:
    {
        ui.stackedWidget->setCurrentWidget( ui.updateDialogPage );
        ui.updateDialogLabel->setText(
            qtr( "Checking for updates..." ) );
        break;
    }
    case UpdateModel::Unchecked:
        // do nothing
        break;
    case UpdateModel::Downloading:
        // NOTE: It is not planned to implement this in the legacy dialog.
        //       The new update pane already respects it, we are only
        //       waiting for the core to provide this information.
        break;
    }

    if (isVisible())
    {
        if (const auto w = windowHandle())
        {
            w->alert(0);
        }
    }
}

#endif
