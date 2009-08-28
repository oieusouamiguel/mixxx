// library.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QItemSelectionModel>

#include "trackinfoobject.h"
#include "durationdelegate.h"
#include "library/library.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/sidebarmodel.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "library/rhythmboxfeature.h"
#include "library/mixxxlibraryfeature.h"
#include "library/playlistfeature.h"

#include "wtracktableview.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

Library::Library(QObject* parent, ConfigObject<ConfigValue>* pConfig)
    : m_pConfig(pConfig) {
    m_pTrackCollection = new TrackCollection();
    m_pSidebarModel = new SidebarModel(parent);
    // TODO -- turn this construction / adding of features into a static method
    // or something -- CreateDefaultLibrary
    addFeature(new MixxxLibraryFeature(this, m_pTrackCollection));
    addFeature(new PlaylistFeature(this, m_pTrackCollection));
    addFeature(new RhythmboxFeature(this));
}

Library::~Library() {
    delete m_pSidebarModel;
    delete m_pTrackCollection;
}

void Library::bindWidget(WLibrarySidebar* pSidebarWidget,
                         WLibrary* pLibraryWidget) {

    WTrackTableView* pTrackTableView =
        new WTrackTableView(pLibraryWidget, m_pConfig);
    connect(this, SIGNAL(searchCleared()),
            pTrackTableView, SLOT(restoreVScrollBarPos()));
	connect(this, SIGNAL(searchStarting()),
            pTrackTableView, SLOT(saveVScrollBarPos()));
    connect(this, SIGNAL(showTrackModel(QAbstractItemModel*)),
            pTrackTableView, SLOT(loadTrackModel(QAbstractItemModel*)));
    connect(pTrackTableView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SLOT(slotLoadTrack(TrackInfoObject*)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SLOT(slotLoadTrackToPlayer(TrackInfoObject*, int)));
    pLibraryWidget->registerView("TRACK", pTrackTableView);
    pLibraryWidget->switchToView("TRACK");

    // Setup the sources view
    pSidebarWidget->setModel(m_pSidebarModel);
    connect(pSidebarWidget, SIGNAL(clicked(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));
    connect(pSidebarWidget, SIGNAL(activated(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));

    // Enable the default selection
    pSidebarWidget->selectionModel()
        ->select(m_pSidebarModel->getDefaultSelection(),
                 QItemSelectionModel::SelectCurrent);
    m_pSidebarModel->activateDefaultSelection();

}

void Library::addFeature(LibraryFeature* feature) {
    m_features.push_back(feature);
    m_pSidebarModel->addLibraryFeature(feature);
    connect(feature, SIGNAL(showTrackModel(QAbstractItemModel*)),
            this, SLOT(slotShowTrackModel(QAbstractItemModel*)));
}

void Library::slotShowTrackModel(QAbstractItemModel* model) {
    qDebug() << "Library::slotShowTrackModel" << model;
    emit(showTrackModel(model));
}

void Library::slotSearch(const QString& text) {
    qDebug() << "Library::search" << text;
    emit(search(text));
}

void Library::slotSearchCleared() {
    qDebug() << "Library::searchCleared";
    emit(searchCleared());
}

void Library::slotSearchStarting() {
    qDebug() << "Library::searchStarting";
    emit(searchStarting());
}

void Library::slotLoadTrack(TrackInfoObject* pTrack) {
    emit(loadTrack(pTrack));
}

void Library::slotLoadTrackToPlayer(TrackInfoObject* pTrack, int player) {
    emit(loadTrackToPlayer(pTrack, player));
}
