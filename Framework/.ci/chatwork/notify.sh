#!/usr/bin/env zsh

set -ue

# Unicode の U+1F300 〜 U+1F6FF から選定（一部豆腐を含む）
# https://ja.wikipedia.org/wiki/Unicode%E3%81%AEEmoji%E3%81%AE%E4%B8%80%E8%A6%A7
emoji=$(echo "\U$(printf '%x' $((RANDOM%1024+127744)))")

# スクリプト実行時の第一引数からタイトルを設定する
# 正しくない値の場合は後々に RESULT 変数が定義されていないエラーで強制終了する
if [ "$1" = 'start' ]; then
  title='👷‍♂️ ジョブを開始しました'
elif [ "$1" = 'success' ]; then
  title='✅ ジョブが成功しました'
elif [ "$1" = 'unstable' ]; then
  title='⚠️ ジョブが不安定です'
elif [ "$1" = 'failure' ]; then
  title='❌ ジョブが失敗しました'
elif [ "$1" = 'aborted' ]; then
  title='🚫 ジョブが中断されました'
fi

# Git コマンドを用いて必要なリポジトリの情報を取得
# Jenkins で定義されている情報が少ないので手動で定義する
# リポジトリ設定情報からリポジトリ名の取得
repository_name=$(basename -s .git "$(git config --get remote.origin.url)")
# コミットメッセージを取得
commit_message=$(git show -s --format=%s)

# Chatwork に通知用のメッセージを作成
# Chatwork 装飾リファレンス
# https://help.chatwork.com/hc/ja/articles/203127904-%E6%8A%95%E7%A8%BF%E3%81%AB%E8%A3%85%E9%A3%BE%E3%81%AF%E3%81%A7%E3%81%8D%E3%81%BE%E3%81%99%E3%81%8B-
# info タグを用いて装飾を行う
# BUILD_DISPLAY_NAME -> Jenkins で定義されたジョブの識別情報
# RUN_DISPLAY_URL -> Jenkins で定義されたジョブの URL リンク
message="[info]
[title]${title} ${emoji}[/title]
🆔 $BUILD_DISPLAY_NAME
🏠 $repository_name
🌵 $BRANCH_NAME
💬 $commit_message
$RUN_DISPLAY_URL
[/info]"


echo '[Script] Post message to chatwork.'
echo "$message"

# curl を用いて Chatwork API に対して投稿を行う
# NOTE: Jenkins の　Chatwork plugin はマルチブランチのワークフローに対応していないため使用できない
# 特定のルームにメッセージを送る API リファレンス
# https://developer.chatwork.com/ja/endpoint_rooms.html#POST-rooms-room_id-messages
# トークン情報は Jenkins に格納している資格情報から取得
curl -fsSL -X POST -H "X-ChatWorkToken: $CHATWORK_TOKEN" \
  "https://api.chatwork.com/v2/rooms/$CHATWORK_ROOM_ID_FOR_NOTIFY/messages" \
  -d "body=$message"
