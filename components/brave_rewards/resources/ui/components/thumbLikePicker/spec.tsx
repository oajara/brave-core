// Copyright (c) 2018 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'
import { shallow } from 'enzyme'
import { create } from 'react-test-renderer'
import { TestThemeProvider } from 'brave-ui/theme'
import ThumbLikePicker from './index'

describe('Amount tests', () => {
  const baseComponent = (props?: object) => (
    <TestThemeProvider>
      <ThumbLikePicker id={'thumbLikePicker'} likeAction={0} />
    </TestThemeProvider>
  )

  describe('basic tests', () => {
    it('matches the snapshot', () => {
      const component = baseComponent()
      const tree = create(component).toJSON()
      expect(tree).toMatchSnapshot()
    })

    it('renders the component', () => {
      const wrapper = shallow(baseComponent())
      const assertion = wrapper.find('#thumbLikePicker').length
      expect(assertion).toBe(1)
    })
  })
})
