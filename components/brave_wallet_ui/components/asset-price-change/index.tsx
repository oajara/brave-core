// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

import { StyledWrapper, PriceChange, ArrowDown, ArrowUp } from './style'

export interface Props {
  isDown: boolean
  priceChangePercentage: string
}

export const AssetPriceChange = (props: Props) => {
  const { isDown, priceChangePercentage } = props

  return (
    <StyledWrapper isDown={isDown}>
      {isDown ? <ArrowDown /> : <ArrowUp />}
      <PriceChange>{priceChangePercentage}</PriceChange>
    </StyledWrapper>
  )
}
