/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const root = styled.div`
  color: var(--brave-palette-neutral900);
  font-size: 13px;
  line-height: 18px;
  text-align: left;

  > div {
    background: #e8f4ff;
    padding: 9px 24px;

    .brave-theme-dark & {
      background: #dff0fe;
    }
  }

  a {
    color: var(--brave-palette-blurple500);
    font-weight: 600;
    text-decoration: none;
  }

  .rewards-payment-pending {
    display: flex;
    gap: 8px;

    .icon {
      color: var(--brave-palette-blue500);
      height: 14px;
      width: auto;
      vertical-align: middle;
    }
  }

  .rewards-payment-amount {
    font-weight: 600;

    .plus {
      margin-right: 2px;
    }
  }

  .rewards-payment-completed {
    background: #e7fdea;
    padding: 9px 15px;
    display: flex;
    align-items: center;
    gap: 13px;

    .brave-theme-dark & {
      background: #cbf1d1;
    }

    .icon {
      height: 24px;
      width: auto;
      vertical-align: middle;
    }
  }
`
