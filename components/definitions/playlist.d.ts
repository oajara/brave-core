/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as PlaylistMojo from 'gen/brave/components/playlist/mojom/playlist.mojom.m.js'

declare namespace Playlist {
  export interface ApplicationState {
    playlistData: State|undefined
    playerState: PlayerState|undefined
  }

  export interface State {
    lists : PlaylistMojo.Playlist[]
    currentList: PlaylistMojo.Playlist|undefined
  }

  export interface PlayerState {
    currentItem: PlaylistMojo.PlaylistItem|undefined
  }
}

