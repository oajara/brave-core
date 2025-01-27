/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { action } from 'typesafe-actions'

// Constants
import { types } from '../constants/welcome_types'

// APIs
import * as welcomeUtils from '../welcomeUtils'

export const importBrowserProfileRequested = (sourceBrowserProfileIndex: number) => action(types.IMPORT_BROWSER_DATA_REQUESTED, sourceBrowserProfileIndex)

export const goToTabRequested = (url: string, target: string) => action(types.GO_TO_TAB_REQUESTED, {
  url,
  target
})

export const closeTabRequested = () => action(types.CLOSE_TAB_REQUESTED)

export const changeDefaultSearchProvider = (searchProvider: string) => action(types.CHANGE_DEFAULT_SEARCH_PROVIDER, searchProvider)

export const getSearchEngineProvidersSuccess = (searchProviders: Welcome.SearchEngineEntry[]) => action(types.IMPORT_DEFAULT_SEARCH_PROVIDERS_SUCCESS, searchProviders)

export const getBrowserProfilesSuccess = (browserProfiles: Welcome.BrowserProfile[]) => action(types.IMPORT_BROWSER_PROFILES_SUCCESS, browserProfiles)

export const getSearchEngineProviders = () => welcomeUtils.getSearchEngineProviders()

export const getBrowserProfiles = () => welcomeUtils.getBrowserProfiles()

export const recordP3A = (details: any) => action(types.RECORD_P3A, {
  details
})
