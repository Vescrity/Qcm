#pragma once

#include "qcm_interface/model.h"
#include "ncm/model.h"

DECLARE_CONVERT(QDateTime, ncm::model::Time)
DECLARE_CONVERT(qcm::model::Artist, ncm::model::Artist)
DECLARE_CONVERT(qcm::model::Artist, ncm::model::Song::Ar)
DECLARE_CONVERT(qcm::model::Album, ncm::model::Album)
DECLARE_CONVERT(qcm::model::Playlist, ncm::model::Playlist)
DECLARE_CONVERT(qcm::model::Song, ncm::model::Song)
DECLARE_CONVERT(qcm::model::User, ncm::model::User)
DECLARE_CONVERT(qcm::model::Comment, ncm::model::Comment)
DECLARE_CONVERT(qcm::model::Djradio, ncm::model::Djradio)
DECLARE_CONVERT(qcm::model::Song, ncm::model::SongB)
DECLARE_CONVERT(qcm::model::Program, ncm::model::Program)

namespace ncm
{
using ItemId = qcm::model::ItemId;

auto to_ncm_id(model::IdType, std::string_view) -> qcm::model::ItemId;
auto to_ncm_id(model::IdType, i64) -> qcm::model::ItemId;
auto ncm_id_type(const ItemId&) -> model::IdType;

} // namespace ncm

template<typename T>
    requires std::is_base_of_v<ncm::model::Id, T>
struct Convert<ncm::ItemId, T> {
    static void from(ncm::ItemId& out, const T& in) {
        using namespace ncm::model;
        out = ncm::to_ncm_id(in.id_type, in.as_str());
    }
};

template<typename T>
    requires std::is_base_of_v<ncm::model::Id, T>
struct Convert<T, ncm::ItemId> {
    static void from(T& out, const ncm::ItemId& in) { out.id = in.id().toStdString(); }
};
