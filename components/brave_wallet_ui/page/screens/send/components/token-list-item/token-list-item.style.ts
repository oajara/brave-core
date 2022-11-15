// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import styled from 'styled-components'

// Shared Styles
import { StyledButton } from '../../shared.styles'

import {
  AssetIconProps,
  AssetIconFactory
} from '../../../../../components/shared/style'

export const AssetIcon = AssetIconFactory<AssetIconProps>({
  width: '40px',
  height: 'auto'
})

export const NetworkIconWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  flex-direction: row;
  position: absolute;
  bottom: -3px;
  right: -3px;
  background-color: ${(p) => p.theme.color.background02};
  border-radius: 100%;
  padding: 2px;
`

export const Button = styled(StyledButton)<{ morePadding?: boolean }>`
  --button-background-hover: ${(p) => p.theme.palette.white};
  --button-shadow-hover: 0px 0px 16px rgba(99, 105, 110, 0.18);
  @media (prefers-color-scheme: dark) {
    --button-background-hover: transparent;
    --button-shadow-hover: 0px 0px 16px rgba(0, 0, 0, 0.36);
  }
  background-color: transparent;
  border-radius: 8px;
  justify-content: space-between;
  padding: 8px;
  white-space: nowrap;
  width: 100%;
  margin-bottom: 8px;
  &:hover {
    background-color: var(--button-background-hover);
    box-shadow: var(--button-shadow-hover);
  }
`

export const IconsWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  flex-direction: row;
  position: relative;
  margin-right: 16px;
`
