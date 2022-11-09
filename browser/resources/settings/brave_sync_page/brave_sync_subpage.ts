// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

// @ts-nocheck TODO(petemill): Define types and remove ts-nocheck

import 'chrome://resources/js/util.m.js';

import 'chrome://resources/cr_elements/cr_button/cr_button.js';
import 'chrome://resources/cr_elements/cr_icon_button/cr_icon_button.js';
import 'chrome://resources/cr_elements/icons.html.js';
import 'chrome://resources/cr_elements/cr_shared_style.css.js';
import 'chrome://resources/cr_elements/cr_shared_vars.css.js';
import '../settings_shared.css.js';
import '../settings_vars.css.js';
import '../people_page/sync_controls.js';
import './brave_sync_configure.js';
import './brave_sync_setup.js';

import { assert } from 'chrome://resources/js/assert.m.js';
import { PolymerElement } from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import { I18nMixin } from 'chrome://resources/js/i18n_mixin.js';

import { Route, RouteObserverMixin, Router } from '../router.js';
import { SyncBrowserProxyImpl, StatusAction } from '../people_page/sync_browser_proxy.js';
import { BraveSyncBrowserProxy } from './brave_sync_browser_proxy.js';
import { getTemplate } from './brave_sync_subpage.html.js'

const SettingBraveSyncSubpageBase = I18nMixin(RouteObserverMixin(PolymerElement))

class SettingBraveSyncSubpage extends SettingBraveSyncSubpageBase {
  static get is() {
    return 'settings-brave-sync-subpage'
  }

  static get template() {
    return getTemplate()
  }

  static get properties() {
    return {
      /**
       * Current page status
       * 'configure' | 'setup' | 'spinner'
       * @private
       */
      pageStatus_: {
        type: String,
        value: 'configure',
      },

      /**
      * The current sync preferences, supplied by SyncBrowserProxy.
      * @type {SyncPrefs|undefined}
      */
      syncPrefs: {
        type: Object,
      },

      /** @type {SyncStatus} */
      syncStatus: {
        type: Object,
      },

      /** @private */
      syncDisabledByAdmin_: {
        type: Boolean,
        value: false,
        computed: 'computeSyncDisabledByAdmin_(syncStatus.managed)',
      },

      /** @private */
      hasSyncWordsDecryptionError_ : {
        type: Boolean,
        value: false,
        computed: 'computeHasSyncWordsDecryptionError_(syncStatus.hasSyncWordsDecryptionError)',
      },

      /** @private */
      hasSyncWordsDecryptionErrorUnlockedSs_ : {
        type: Boolean,
        value: false,
        computed: 'computeHasSyncWordsDecryptionErrorUnlockedSs_(syncStatus.hasSyncWordsDecryptionError)',
      },

      /** @private */
      syncSectionDisabled_: {
        type: Boolean,
        value: false,
        computed: 'computeSyncSectionDisabled_(' +
          'syncStatus.disabled, ' +
          'syncStatus.hasError, syncStatus.statusAction, ' +
          'syncPrefs.trustedVaultKeysRequired)',
      },
    }
  }

  static get observers() {
    return [
      'updatePageStatus_(syncStatus.*)'
    ]
  }

  /** @private {?SyncBrowserProxy} */
  browserProxy_ = SyncBrowserProxyImpl.getInstance()

  braveSyncBrowserProxy_ = BraveSyncBrowserProxy.getInstance();

  /**
  * The beforeunload callback is used to show the 'Leave site' dialog. This
  * makes sure that the user has the chance to go back and confirm the sync
  * opt-in before leaving.
  *
  * This property is non-null if the user is currently navigated on the sync
  * settings route.
  *
  * @private {?Function}
  */
  beforeunloadCallback_ = null

  /**
  * The unload callback is used to cancel the sync setup when the user hits
  * the browser back button after arriving on the page.
  * Note: Cases like closing the tab or reloading don't need to be handled,
  * because they are already caught in |PeopleHandler::~PeopleHandler|
  * from the C++ code.
  *
  * @private {?Function}
  */
  unloadCallback_ = null

  /**
  * Whether the user completed setup successfully.
  * @private {boolean}
  */
  setupSuccessful_ = false

  attached() {
    super.attached()
    const router = Router.getInstance()
    if (router.getCurrentRoute() == router.getRoutes().BRAVE_SYNC_SETUP) {
      this.onNavigateToPage_()
    }
  }

  /** @override */
  detached() {
    const router = Router.getInstance()
    if (router.getRoutes().BRAVE_SYNC_SETUP.contains(router.getCurrentRoute())) {
      this.onNavigateAwayFromPage_()
    }

    if (this.beforeunloadCallback_) {
      window.removeEventListener('beforeunload', this.beforeunloadCallback_)
      this.beforeunloadCallback_ = null
    }
    if (this.unloadCallback_) {
      window.removeEventListener('unload', this.unloadCallback_)
      this.unloadCallback_ = null
    }

    super.detached()
  }

  updatePageStatus_(newValue, oldValue) {
console.log("brave_sync_subpage.ts:updatePageStatus_ 000")
    const isFirstSetup = this.syncStatus && this.syncStatus.firstSetupInProgress
    this.pageStatus_ = isFirstSetup ? 'setup' : 'configure'
  }

  /**
  * @return {boolean}
  * @private
  */
  computeSyncSectionDisabled_() {
    return this.syncStatus !== undefined &&
      (!!this.syncStatus.disabled ||
        !!this.syncStatus.hasSyncWordsDecryptionError ||
        (!!this.syncStatus.hasError &&
          this.syncStatus.statusAction !==
          StatusAction.ENTER_PASSPHRASE &&
          this.syncStatus.statusAction !==
          StatusAction.RETRIEVE_TRUSTED_VAULT_KEYS))
  }

  /**
  * @return {boolean}
  * @private
  */
  computeSyncDisabledByAdmin_() {
    return this.syncStatus != undefined && !!this.syncStatus.managed
  }

  /**
  * @return {boolean}
  * @private
  */
  computeHasSyncWordsDecryptionError_() {
console.log("brave_sync_subpage.ts:computeHasSyncWordsDecryptionError_ 000")
    // return this.syncStatus != undefined && !!this.syncStatus.hasSyncWordsDecryptionError
    //   && !this.syncStatus.isOsEncryptionAvailable
    const bret = this.syncStatus != undefined && !!this.syncStatus.hasSyncWordsDecryptionError
      && !this.syncStatus.isOsEncryptionAvailable
    console.log("computeHasSyncWordsDecryptionError_ EXIT bret="+bret)
  }

  /**
  * @return {boolean}
  * @private
  */  
  computeHasSyncWordsDecryptionErrorUnlockedSs_() {
    console.log("brave_sync_subpage.ts:computeHasSyncWordsDecryptionErrorUnlockedSs_ 000")
    console.log("this.syncStatus=",this.syncStatus)
    if (this.syncStatus) {
      console.log("this.syncStatus.hasSyncWordsDecryptionError=",this.syncStatus.hasSyncWordsDecryptionError)
      console.log("this.syncStatus.isOsEncryptionAvailable=",this.syncStatus.isOsEncryptionAvailable)
    }
    // return this.syncStatus != undefined && !!this.syncStatus.hasSyncWordsDecryptionError
    //   && !!this.syncStatus.isOsEncryptionAvailable
    const bret = this.syncStatus != undefined && !!this.syncStatus.hasSyncWordsDecryptionError
      && !!this.syncStatus.isOsEncryptionAvailable
    
    console.log("computeHasSyncWordsDecryptionErrorUnlockedSs_ EXIT bret="+bret)
    return bret
  }

  /** @protected */
  currentRouteChanged() {
    const router = Router.getInstance();
    if (router.getCurrentRoute() == router.getRoutes().BRAVE_SYNC_SETUP) {
      this.onNavigateToPage_()
      return
    }

    if (router.getRoutes().BRAVE_SYNC_SETUP.contains(router.getCurrentRoute())) {
      return
    }

    this.onNavigateAwayFromPage_()
  }

  /**
  * @param {!PageStatus} expectedPageStatus
  * @return {boolean}
  * @private
  */
  isStatus_(expectedPageStatus) {
    return expectedPageStatus == this.pageStatus_;
  }

  /** @private */
  onNavigateToPage_() {
    const router = Router.getInstance()
    assert(router.getCurrentRoute() == router.getRoutes().BRAVE_SYNC_SETUP)
    if (this.beforeunloadCallback_) {
      return
    }

    // Triggers push of prefs to our handler
    this.browserProxy_.didNavigateToSyncPage()

    this.beforeunloadCallback_ = event => {
      // When the user tries to leave the sync setup, show the 'Leave site'
      // dialog.
      if (this.syncStatus && this.syncStatus.firstSetupInProgress) {
        event.preventDefault()
        event.returnValue = ''

        chrome.metricsPrivate.recordUserAction(
          'Signin_Signin_AbortAdvancedSyncSettings')
      }
    };
    window.addEventListener('beforeunload', this.beforeunloadCallback_)

    this.unloadCallback_ = this.onNavigateAwayFromPage_.bind(this)
    window.addEventListener('unload', this.unloadCallback_)
  }

  /** @private */
  onNavigateAwayFromPage_() {
    if (!this.beforeunloadCallback_) {
      return
    }

    this.browserProxy_.didNavigateAwayFromSyncPage(!this.setupSuccessful_)

    // Reset state as this component could actually be kept around even though
    // it is hidden.
    this.setupSuccessful_ = false

    window.removeEventListener('beforeunload', this.beforeunloadCallback_)
    this.beforeunloadCallback_ = null

    if (this.unloadCallback_) {
      window.removeEventListener('unload', this.unloadCallback_)
      this.unloadCallback_ = null
    }
  }

  /**
  * Called when setup is complete and sync code is set
  * @private
  */
  onSetupSuccess_() {
    this.setupSuccessful_ = true
    // This navigation causes the firstSetupInProgress flag to be marked as false
    // via `didNavigateAwayFromSyncPage`.
    const router = Router.getInstance()
    if (router.getCurrentRoute() == router.getRoutes().BRAVE_SYNC_SETUP) {
      router.navigateTo(router.getRoutes().BRAVE_SYNC)
    }
  }

  /**
  * Called when we see that OS safe storage is not locked, but we can't decrypt 
  * the passphrase. So we proposing user to clear data and re-joid the chain.
  */
  onRejoin_ = async function (e) {
console.log("brave_sync_subpage.ts:onRejoin_ 000")
    const messageText = this.i18n('braveSyncResetConfirmation')
    const shouldReset = confirm(messageText)
    if (!shouldReset) {
      return
    }
console.log("brave_sync_subpage.ts:onRejoin_ 001")
    this.pageStatus_ = 'spinner'
    await this.braveSyncBrowserProxy_.stopSyncClearData();
console.log("brave_sync_subpage.ts:onRejoin_ 002")
     window.setTimeout(() => {
 console.log("brave_sync_subpage.ts:onRejoin_ 003")
// 
//       const router = Router.getInstance()
//       router.navigateTo(router.getRoutes().BRAVE_SYNC) // better, but not perfect
//       //router.navigateTo(router.getRoutes().BRAVE_SYNC_SETUP) // not good at all
// 
//       this.pageStatus_ = 'setup' // this has no effect
//console.log("this.shadowRoot=", this.shadowRoot)

//this.fire('show-get-code') crash

       //elem = document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("div > div.content > div > cr-button:nth-child(2)")
//console.log("elem=",elem)
console.log("1=",document.querySelector("body > settings-ui"))
console.log("2=",document.querySelector("body > settings-ui").shadowRoot)
console.log("3=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main"))
console.log("4=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot)
console.log("5=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page"))
console.log("6=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot)
console.log("7=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page"))
console.log("8=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot)
console.log("9=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage"))
console.log("A=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot)
console.log("B=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot.querySelector("#sync-section > settings-brave-sync-setup"))
console.log("C=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot)
console.log("D=",document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("div > div.content > div > cr-button:nth-child(2)"))

console.log("brave_sync_subpage.ts:onRejoin_ 003-5")
console.log("10=",this);
console.log("11=",this.shadowRoot);
console.log("12=",this.shadowRoot.querySelector("#sync-section > settings-brave-sync-setup"))
console.log("13=",this.shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot)
console.log("14=",this.shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("div > div.content > div > cr-button:nth-child(2)"))
console.log("brave_sync_subpage.ts:onRejoin_ 003-6")

let elem1 = this.shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("div > div.content > div > cr-button:nth-child(2)")
console.log("elem1=",elem1);
let elem2 = document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("div > div.content > div > cr-button:nth-child(2)")
console.log("elem2=",elem2);

let elem3 = document.querySelector("body > settings-ui").shadowRoot.querySelector("#main").shadowRoot.querySelector("settings-basic-page").shadowRoot.querySelector("#basicPage > settings-section.expanded > settings-brave-sync-page").shadowRoot.querySelector("#pages > settings-subpage > settings-brave-sync-subpage").shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("#enterSyncCodeButton")
console.log("elem3=",elem3);

let elem4 = document.getElementById('enterSyncCodeButton') // null
console.log("elem4=",elem4);

let elem5 = this.shadowRoot.querySelector("#sync-section > settings-brave-sync-setup").shadowRoot.querySelector("#enterSyncCodeButton")
console.log("elem4=",elem5);

console.log("brave_sync_subpage.ts:onRejoin_ 003-7")

// const event = new MouseEvent('click', {
//     view: window,
//     bubbles: true,
//     cancelable: true
//   }); //ok
const event = new MouseEvent('click'); //ok

console.log("brave_sync_subpage.ts:onRejoin_ 003-8")
elem5.dispatchEvent(event);
console.log("brave_sync_subpage.ts:onRejoin_ 003-9")

/*
example of is assigning search shadowRoot
saveButton.id = 'saveOnExitSettingsConfirm';


this.shadowRoot.querySelector('#saveOnExitSettingsConfirm')



dispatchEvent https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/dispatchEvent

const event = document.createEvent('MouseEvent');
event.initMouseEvent(
    'click', true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
a.dispatchEvent(event);

https://developer.mozilla.org/en-US/docs/Web/Events/Creating_and_triggering_events#triggering_built-in_events

*/

//       //window.location.reload(); - causes additional reload
//       window.setTimeout(() => {
//         const router = Router.getInstance()
//         router.navigateTo(router.getRoutes().BRAVE_SYNC_SETUP)
//     },0)
}, 1000)
console.log("brave_sync_subpage.ts:onRejoin_ 004")
  }
}

customElements.define(
  SettingBraveSyncSubpage.is, SettingBraveSyncSubpage)
