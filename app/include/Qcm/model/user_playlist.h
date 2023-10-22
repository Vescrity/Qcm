#pragma once

#include <QQmlEngine>
#include <QAbstractListModel>

#include "Qcm/api.h"
#include "Qcm/model.h"
#include "ncm/api/user_playlist.h"

#include "meta_model/qgadgetlistmodel.h"

#include "core/log.h"

namespace qcm
{
namespace model
{
struct UserPlaylistItem {
    Q_GADGET
public:
    GATGET_PROPERTY(PlaylistId, itemId, itemId)
    GATGET_PROPERTY(QString, name, name)
    GATGET_PROPERTY(QString, picUrl, picUrl)
    GATGET_PROPERTY(qint32, playCount, playCount)
    GATGET_PROPERTY(qint32, trackCount, trackCount)

    std::strong_ordering operator<=>(const UserPlaylistItem&) const = default;
};
} // namespace model
} // namespace qcm

DEFINE_CONVERT(qcm::model::UserPlaylistItem, ncm::model::UserPlaylistItem) {
    convert(out.itemId, in.id);
    convert(out.name, in.name);
    convert(out.picUrl, in.coverImgUrl);
    convert(out.playCount, in.playCount);
    convert(out.trackCount, in.trackCount);
}

namespace qcm
{
namespace model
{

class UserPlaylist : public meta_model::QGadgetListModel<UserPlaylistItem> {
    Q_OBJECT
public:
    UserPlaylist(QObject* parent = nullptr)
        : meta_model::QGadgetListModel<UserPlaylistItem>(parent), m_has_more(true) {}
    using out_type = ncm::api_model::UserPlaylist;

    void handle_output(const out_type& re, const auto& input) {
        if (input.offset == 0) {
            auto in_ = convert_from<std::vector<UserPlaylistItem>>(re.playlist);
            convertModel(in_, [](const UserPlaylistItem& it) {
                return convert_from<std::string>(it.itemId);
            });
            m_has_more = re.more;
        } else if (input.offset == (int)rowCount()) {
            for (auto& el : re.playlist) {
                insert(rowCount(), convert_from<UserPlaylistItem>(el));
            }
            m_has_more = re.more;
        }
    }

    bool canFetchMore(const QModelIndex&) const override { return m_has_more; }
    void fetchMore(const QModelIndex&) override {
        m_has_more = false;
        emit fetchMoreReq(rowCount());
    }
signals:
    void fetchMoreReq(qint32);

private:
    bool m_has_more;
};
static_assert(modelable<UserPlaylist, ncm::api::UserPlaylist>);
} // namespace model

using UserPlaylistQuerier_base = ApiQuerier<ncm::api::UserPlaylist, model::UserPlaylist>;
class UserPlaylistQuerier : public UserPlaylistQuerier_base {
    Q_OBJECT
    QML_ELEMENT
public:
    UserPlaylistQuerier(QObject* parent = nullptr): UserPlaylistQuerier_base(parent) {}

    FORWARD_PROPERTY(model::UserId, uid, uid)
    FORWARD_PROPERTY(qint32, offset, offset)
    FORWARD_PROPERTY(qint32, limit, limit)

public:
    void fetch_more(qint32 cur_count) override { set_offset(cur_count); }
public slots:
    void reset() {
        api().input.offset = 0;
        reload();
    }
};

} // namespace qcm
