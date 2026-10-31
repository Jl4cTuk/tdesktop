/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "data/data_hidden_topics.h"

#include "data/data_forum.h"
#include "data/data_forum_topic.h"
#include "data/data_peer.h"
#include "settings.h"

#include <QtCore/QFile>
#include <QtCore/QSaveFile>

namespace Data {
namespace {

// PeerId value and topic root message id.
using HiddenTopic = std::pair<uint64, int64>;

[[nodiscard]] QString HiddenTopicsPath() {
	return cWorkingDir() + u"tdata/hidden_topics"_q;
}

[[nodiscard]] base::flat_set<HiddenTopic> Load() {
	auto result = base::flat_set<HiddenTopic>();
	auto file = QFile(HiddenTopicsPath());
	if (!file.open(QIODevice::ReadOnly)) {
		return result;
	}
	for (const auto &line : file.readAll().split('\n')) {
		const auto parts = line.trimmed().split(':');
		if (parts.size() != 2) {
			continue;
		}
		auto peerOk = false;
		auto rootOk = false;
		const auto peer = parts[0].toULongLong(&peerOk);
		const auto root = parts[1].toLongLong(&rootOk);
		if (peerOk && rootOk) {
			result.emplace(peer, root);
		}
	}
	return result;
}

[[nodiscard]] base::flat_set<HiddenTopic> &List() {
	static auto result = Load();
	return result;
}

void Save() {
	auto text = QByteArray();
	for (const auto &[peer, root] : List()) {
		text += QByteArray::number(peer)
			+ ':'
			+ QByteArray::number(root)
			+ '\n';
	}
	auto file = QSaveFile(HiddenTopicsPath());
	if (file.open(QIODevice::WriteOnly)) {
		file.write(text);
		file.commit();
	}
}

} // namespace

bool IsTopicHidden(PeerId peerId, MsgId rootId) {
	return List().contains(HiddenTopic(peerId.value, rootId.bare));
}

int HiddenTopicsCount(PeerId peerId) {
	return ranges::count(List(), peerId.value, &HiddenTopic::first);
}

void HideTopic(not_null<ForumTopic*> topic) {
	const auto key = HiddenTopic(
		topic->peer()->id.value,
		topic->rootId().bare);
	if (!List().emplace(key).second) {
		return;
	}
	Save();
	topic->updateChatListExistence();
	topic->forum()->hiddenTopicsChanged();
}

void ShowHiddenTopics(not_null<Forum*> forum) {
	const auto peerId = forum->peer()->id.value;
	auto &list = List();
	auto roots = std::vector<MsgId>();
	for (const auto &[peer, root] : list) {
		if (peer == peerId) {
			roots.push_back(MsgId(root));
		}
	}
	if (roots.empty()) {
		return;
	}
	for (const auto rootId : roots) {
		list.remove(HiddenTopic(peerId, rootId.bare));
	}
	Save();
	for (const auto rootId : roots) {
		if (const auto topic = forum->topicFor(rootId)) {
			// While hidden its sort position was dropped, so recompute it,
			// that puts the topic back to the list.
			topic->updateChatListSortPosition();
		}
	}
	forum->hiddenTopicsChanged();
}

} // namespace Data
