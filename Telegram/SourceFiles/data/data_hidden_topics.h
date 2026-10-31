/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Data {

class Forum;
class ForumTopic;

// Forum topics hidden by the user. A hidden topic is left out of the
// forum's topics list, so it disappears from the topics list, the topic
// tabs, the chats list row preview and the forum's unread counters, while
// it still opens from a link or a notification.
//
// The list is kept locally in one file for all accounts: channel peer ids
// are the same in every account.
[[nodiscard]] bool IsTopicHidden(PeerId peerId, MsgId rootId);
[[nodiscard]] int HiddenTopicsCount(PeerId peerId);
void HideTopic(not_null<ForumTopic*> topic);
void ShowHiddenTopics(not_null<Forum*> forum);

} // namespace Data
