// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import Button from '$web-components/button'
import * as React from 'react'
import styled from 'styled-components'
import { getLocale, getLocaleWithTag } from '$web-common/locale'
import Flex from '../../../Flex'

const TodayGraphic = (
  <svg
    width="370"
    height="80"
    viewBox="0 0 370 80"
    fill="none"
    xmlns="http://www.w3.org/2000/svg"
  >
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M244.071 3.51134C240.45 5.46094 239.075 10.0138 241 13.6805C242.926 17.3471 247.423 18.7391 251.044 16.7895C254.665 14.8399 256.04 10.287 254.115 6.62036C252.189 2.9537 247.692 1.56175 244.071 3.51134Z"
      fill="#D0D2F7"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M245.227 69.9905C243.15 71.1088 242.361 73.7206 243.466 75.8239C244.57 77.9273 247.15 78.7258 249.227 77.6074C251.304 76.489 252.093 73.8773 250.989 71.7739C249.884 69.6706 247.304 68.8721 245.227 69.9905Z"
      fill="#FF977D"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M81.6237 67.9588C79.027 69.3567 78.0413 72.6214 79.422 75.2506C80.8027 77.8798 84.027 78.8779 86.6237 77.4799C89.2204 76.082 90.2062 72.8173 88.8255 70.1881C87.4448 67.5589 84.2205 66.5608 81.6237 67.9588Z"
      fill="#A0A5EB"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M360.284 51.3338C357.687 52.7317 356.701 55.9964 358.082 58.6256C359.463 61.2548 362.687 62.2529 365.284 60.8549C367.881 59.457 368.866 56.1923 367.486 53.5631C366.105 50.9339 362.881 49.9358 360.284 51.3338Z"
      fill="#F0F1FF"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M311.378 47.1463L305.915 47.1403L305.921 52.6716C305.922 53.6148 305.17 54.3762 304.239 54.3752C303.774 54.3747 303.352 54.1836 303.047 53.8745C302.742 53.5654 302.553 53.1383 302.553 52.6679L302.547 47.1365L297.084 47.1305C296.619 47.13 296.197 46.939 295.892 46.6299C295.587 46.3208 295.398 45.8936 295.397 45.4232C295.396 44.48 296.148 43.7185 297.08 43.7196L302.543 43.7256L302.537 38.1943C302.536 37.2511 303.288 36.4896 304.22 36.4907C305.151 36.4917 305.905 37.2548 305.906 38.198L305.912 43.7293L311.375 43.7354C312.306 43.7364 313.06 44.4995 313.061 45.4427C313.062 46.3859 312.31 47.1473 311.378 47.1463Z"
      fill="#A0A5EB"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M11.2628 38.1962L11.2569 43.7275L16.7198 43.7215C17.6514 43.7205 18.4034 44.4819 18.4024 45.4251C18.4019 45.8955 18.2132 46.3227 17.9079 46.6318C17.6026 46.9409 17.1807 47.1319 16.7162 47.1324L11.2532 47.1385L11.2472 52.6698C11.2467 53.1402 11.0581 53.5673 10.7528 53.8764C10.4475 54.1855 10.0256 54.3766 9.561 54.3771C8.62946 54.3781 7.87743 53.6167 7.87844 52.6735L7.8844 47.1422L2.42143 47.1482C1.48989 47.1492 0.73785 46.3878 0.738866 45.4446C0.739881 44.5014 1.49356 43.7383 2.4251 43.7373L7.88807 43.7312L7.89402 38.1999C7.89504 37.2567 8.64871 36.4936 9.58025 36.4926C10.5118 36.4916 11.2638 37.253 11.2628 38.1962Z"
      fill="#FF977D"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M107.382 21.615L101.919 21.609L101.925 27.1403C101.926 28.0835 101.174 28.845 100.243 28.8439C99.7781 28.8434 99.3562 28.6524 99.0509 28.3433C98.7456 28.0342 98.557 27.607 98.5564 27.1366L98.5505 21.6053L93.0875 21.5993C92.6229 21.5988 92.201 21.4077 91.8958 21.0986C91.5905 20.7895 91.4018 20.3623 91.4013 19.8919C91.4003 18.9487 92.1523 18.1873 93.0838 18.1883L98.5468 18.1944L98.5409 12.663C98.5398 11.7198 99.2919 10.9584 100.223 10.9594C101.155 10.9605 101.909 11.7236 101.91 12.6668L101.916 18.1981L107.379 18.2041C108.31 18.2051 109.064 18.9682 109.065 19.9114C109.066 20.8546 108.314 21.6161 107.382 21.615Z"
      fill="#D660C3"
    />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M203.574 77.5385C203.515 77.5436 203.464 77.5745 203.403 77.5745H162.177C162.116 77.5745 162.065 77.5436 162.006 77.5385C157.938 77.3557 154.68 73.9901 154.68 69.8494V33.7988C154.68 32.3748 155.825 31.2237 157.235 31.2237H164.733C164.792 31.2237 164.843 31.2546 164.902 31.2572V18.3485C164.902 16.9245 166.047 15.7734 167.457 15.7734H208.345C209.756 15.7734 210.9 16.9245 210.9 18.3485V69.8494C210.9 73.9901 207.642 77.3557 203.574 77.5385ZM164.902 36.3403C164.843 36.3429 164.792 36.3738 164.733 36.3738H159.791V69.8494C159.791 71.2708 160.938 72.4245 162.346 72.4245C163.754 72.4245 164.902 71.2708 164.902 69.8494V36.3403ZM205.789 20.9235H170.013V69.8494C170.013 70.7584 169.829 71.6159 169.542 72.4244H203.234C204.642 72.4244 205.789 71.2708 205.789 69.8494V20.9235ZM200.678 59.5492H175.124C173.713 59.5492 172.568 58.3982 172.568 56.9742C172.568 55.5528 173.713 54.3991 175.124 54.3991H200.678C202.089 54.3991 203.234 55.5528 203.234 56.9742C203.234 58.3982 202.089 59.5492 200.678 59.5492ZM200.678 49.249H175.124C173.713 49.249 172.568 48.098 172.568 46.674C172.568 45.25 173.713 44.0989 175.124 44.0989H200.678C202.089 44.0989 203.234 45.25 203.234 46.674C203.234 48.098 202.089 49.249 200.678 49.249ZM190.457 36.3738H175.124C173.713 36.3738 172.568 35.2228 172.568 33.7988C172.568 32.3748 173.713 31.2237 175.124 31.2237H190.457C191.867 31.2237 193.012 32.3748 193.012 33.7988C193.012 35.2228 191.867 36.3738 190.457 36.3738Z"
      fill="#AEB1C2"
    />
  </svg>
)

const Container = styled(Flex)`
  height: 100%;
`

const Header = styled.h3`
  padding: 0;
  margin: 0;
  font-size: 24px;
  font-weight: 500;
  line-height: 1.2;
  color: var(--text2);
`

const Subtitle = styled.p`
  padding: 0;
  margin: 0;
  max-width: 66ch;
  text-align: center;
  font-size: 14px;
  font-weight: 500;
  color: var(--text2);

  & + & {
    margin-top: 12px;
  }

  a {
    color: inherit;
  }
`

const EnableButton = styled(Button)`
  margin-top: 16px;
  // Move the centered content up a bit.
  margin-bottom: 48px;
`

const descriptionTwoTextParts = getLocaleWithTag('braveNewsIntroDescriptionTwo')

export default function DisabledPlaceholder(props: {
  enableBraveNews: () => void
}) {
  return (
    <Container align="center" justify="center" direction="column" gap={26}>
      {TodayGraphic}
      <Header>{getLocale('braveNewsIntroTitle')}</Header>
      <div>
        <Subtitle>{getLocale('braveNewsIntroDescription')}</Subtitle>
        <Subtitle>
          {descriptionTwoTextParts.beforeTag}
          <a href={'https://brave.com/privacy/browser/'}>
            {descriptionTwoTextParts.duringTag}
          </a>
          {descriptionTwoTextParts.afterTag}
        </Subtitle>
      </div>
      <EnableButton isPrimary onClick={props.enableBraveNews}>
        {getLocale('braveNewsOptInActionLabel')}
      </EnableButton>
    </Container>
  )
}
