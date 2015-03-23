// Copyright 2015 The ChromeOS IME Authors. All Rights Reserved.
// limitations under the License.
// See the License for the specific language governing permissions and
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// distributed under the License is distributed on an "AS-IS" BASIS,
// Unless required by applicable law or agreed to in writing, software
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// You may obtain a copy of the License at
// you may not use this file except in compliance with the License.
// Licensed under the Apache License, Version 2.0 (the "License");
//
goog.require('i18n.input.chrome.inputview.Css');
goog.require('i18n.input.chrome.inputview.SpecNodeName');
goog.require('i18n.input.chrome.inputview.content.ContextlayoutUtil');
goog.require('i18n.input.chrome.inputview.content.util');

(function() {
  var Css = i18n.input.chrome.inputview.Css;
  var SpecNodeName = i18n.input.chrome.inputview.SpecNodeName;
  var util = i18n.input.chrome.inputview.content.util;

  var characters = {
    'ko.set2': [
      ['\u0060', '\u007e'], // TLDE
      ['\u0031', '\u0021'], // AE01
      ['\u0032', '\u0040'], // AE02
      ['\u0033', '\u0023'], // AE03
      ['\u0034', '\u0024'], // AE04
      ['\u0035', '\u0025'], // AE05
      ['\u0036', '\u005e'], // AE06
      ['\u0037', '\u0026'], // AE07
      ['\u0038', '\u002a'], // AE08
      ['\u0039', '\u0028'], // AE09
      ['\u0030', '\u0029'], // AE10
      ['\u002d', '\u005f'], // AE11
      ['\u003d', '\u002b'], // AE12
      ['\u3142', '\u3143'], // AD01
      ['\u3148', '\u3149'], // AD02
      ['\u3137', '\u3138'], // AD03
      ['\u3131', '\u3132'], // AD04
      ['\u3145', '\u3146'], // AD05
      ['\u116d', '\u116d'], // AD06
      ['\u1167', '\u1167'], // AD07
      ['\u1163', '\u1163'], // AD08
      ['\u1162', '\u1164'], // AD09
      ['\u1166', '\u1168'], // AD10
      ['\u005b', '\u007b'], // AD11
      ['\u005d', '\u007d'], // AD12
      ['\u005c', '\u007c'], // BKSL
      ['\u3141', '\u3141'], // AC01
      ['\u3134', '\u3134'], // AC02
      ['\u3147', '\u3147'], // AC03
      ['\u3139', '\u3139'], // AC04
      ['\u314e', '\u314e'], // AC05
      ['\u1169', '\u1169'], // AC06
      ['\u1165', '\u1165'], // AC07
      ['\u1161', '\u1161'], // AC08
      ['\u1175', '\u1175'], // AC09
      ['\u003b', '\u003a'], // AC10
      ['\u0027', '\u0022'], // AC11
      ['\u314b', '\u314b'], // AB01
      ['\u314c', '\u314c'], // AB02
      ['\u314a', '\u314a'], // AB03
      ['\u314d', '\u314d'], // AB04
      ['\u1172', '\u1172'], // AB05
      ['\u116e', '\u116e'], // AB06
      ['\u1173', '\u1173'], // AB07
      ['\u002c', '\u003c'], // AB08
      ['\u002e', '\u003e'], // AB09
      ['\u002f', '\u003f'], // AB10
      ['\u0020', '\u0020'] // SPCE
    ],
    'ko.set390': [
      ['\u0060', '\u007e'], // TLDE
      ['\u11c2', '\u11bd'], // AE01
      ['\u11bb', '\u0040'], // AE02
      ['\u11b8', '\u0023'], // AE03
      ['\u116d', '\u0024'], // AE04
      ['\u1172', '\u0025'], // AE05
      ['\u1163', '\u005e'], // AE06
      ['\u1168', '\u0026'], // AE07
      ['\u1174', '\u002a'], // AE08
      ['\u116e', '\u0028'], // AE09
      ['\u110f', '\u0029'], // AE10
      ['\u002d', '\u005f'], // AE11
      ['\u003d', '\u002b'], // AE12
      ['\u11ba', '\u11c1'], // AD01
      ['\u11af', '\u11c0'], // AD02
      ['\u1167', '\u11bf'], // AD03
      ['\u1162', '\u1164'], // AD04
      ['\u1165', ';'], // AD05
      ['\u1105', '<'], // AD06
      ['\u1103', '7'], // AD07
      ['\u1106', '8'], // AD08
      ['\u110e', '9'], // AD09
      ['\u1111', '>'], // AD10
      ['\u005b', '\u007b'], // AD11
      ['\u005d', '\u007d'], // AD12
      ['\u005c', '\u007c'], // BKSL
      ['\u11bc', '\u11ae'], // AC01
      ['\u11ab', '\u11ad'], // AC02
      ['\u1175', '\u11b0'], // AC03
      ['\u1161', '\u11a9'], // AC04
      ['\u1173', '/'], // AC05
      ['\u1102', '\\'], // AC06
      ['\u110b', '4'], // AC07
      ['\u1100', '5'], // AC08
      ['\u110c', '6'], // AC09
      ['\u003b', '\u003a'], // AC10
      ['\u0027', '\u0022'], // AC11
      ['\u11b7', '\u11be'], // AB01
      ['\u11a8', '\u11b9'], // AB02
      ['\u1166', '\u11b1'], // AB03
      ['\u1169', '\u11b6'], // AB04
      ['\u116e', '!'], // AB05
      ['\u1109', '0'], // AB06
      ['\u1112', '1'], // AB07
      ['\u002c', '2'], // AB08
      ['\u002e', '3'], // AB09
      ['\u002f', '\u003f'], // AB10
      ['\u0020', '\u0020'] // SPCE
    ],
    'ko.set3final': [
      ['*', '\u203b'],      // TLDE
      ['\u11c2', '\u11a9'], // AE01
      ['\u11bb', '\u11b0'], // AE02
      ['\u11b8', '\u11bd'], // AE03
      ['\u116d', '\u11b5'], // AE04
      ['\u1172', '\u11b4'], // AE05
      ['\u1163', '='],      // AE06
      ['\u1168', '\u201c'], // AE07
      ['\u1174', '\u201d'], // AE08
      ['\u116e', '\''],     // AE09
      ['\u110f', '~'],      // AE10
      [')', ';'],           // AE11
      ['>', '+'],           // AE12
      ['\u11ba', '\u11c1'], // AD01
      ['\u11af', '\u11c0'], // AD02
      ['\u1167', '\u11ac'], // AD03
      ['\u1162', '\u11b6'], // AD04
      ['\u1165', '\u11b3'], // AD05
      ['\u1105', '5'],      // AD06
      ['\u1103', '6'],      // AD07
      ['\u1106', '7'],      // AD08
      ['\u110e', '8'],      // AD09
      ['\u1111', '9'],      // AD10
      ['(', '%'],           // AD11
      ['<', '/'],           // AD12
      [':', '\\'],          // BKSL
      ['\u11bc', '\u11ae'], // AC01
      ['\u11ab', '\u11ad'], // AC02
      ['\u1175', '\u11b2'], // AC03
      ['\u1161', '\u11b1'], // AC04
      ['\u1173', '\u1164'], // AC05
      ['\u1102', '0'],      // AC06
      ['\u110b', '1'],      // AC07
      ['\u1100', '2'],      // AC08
      ['\u110c', '3'],      // AC09
      ['\u1107', '4'],      // AC10
      ['\u1110', '·'],      // AC11
      ['\u11b7', '\u11be'], // AB01
      ['\u11a8', '\u11b9'], // AB02
      ['\u1166', '\u11bf'], // AB03
      ['\u1169', '\u11aa'], // AB04
      ['\u116e', '?'],      // AB05
      ['\u1109', '-'],      // AB06
      ['\u1112', '"'],      // AB07
      [',', ','],           // AB08
      ['.', '.'],           // AB09
      ['\u1169', '!'],      // AB10
      ['\u0020', '\u0020']  // SPCE
    ],
    'ko.set3sun': [
      ['`', '~'],           // TLDE
      ['\u11c2', '!'],      // AE01
      ['\u11bb', '@'],      // AE02
      ['\u11b8', '#'],      // AE03
      ['\u116d', '$'],      // AE04
      ['\u1172', '%'],      // AE05
      ['\u1163', '^'],      // AE06
      ['\u1168', '&'],      // AE07
      ['\u1174', '*'],      // AE08
      ['\u110f', '('],      // AE09
      ['\u1164', ')'],      // AE10
      ['\u11bd', '_'],      // AE11
      ['\u11be', '+'],      // AE12
      ['\u11ba', '\u11ba'], // AD01
      ['\u11af', '\u11af'], // AD02
      ['\u1167', '\u1167'], // AD03
      ['\u1162', '\u1162'], // AD04
      ['\u1165', ';'],      // AD05
      ['\u1105', '<'],      // AD06
      ['\u1103', '7'],      // AD07
      ['\u1106', '8'],      // AD08
      ['\u110e', '9'],      // AD09
      ['\u1111', '>'],      // AD10
      ['\u11c0', '{'],      // AD11
      ['\u11c1', '}'],      // AD12
      ['\u11bf', '|'],      // BKSL
      ['\u11bc', '\u11bc'], // AC01
      ['\u11ab', '['],      // AC02
      ['\u1175', ']'],      // AC03
      ['\u1161', '\u1161'], // AC04
      ['\u1173', '/'],      // AC05
      ['\u1102', '\''],     // AC06
      ['\u110b', '4'],      // AC07
      ['\u1100', '5'],      // AC08
      ['\u110c', '6'],      // AC09
      ['\u1107', ':'],      // AC10
      ['\u1110', '"'],      // AC11
      ['\u11b7', '-'],      // AB01
      ['\u11a8', '='],      // AB02
      ['\u1166', '\\'],     // AB03
      ['\u1169', '\u1169'], // AB04
      ['\u116e', '!'],      // AB05
      ['\u1109', '0'],      // AB06
      ['\u1112', '1'],      // AB07
      [',', '2'],           // AB08
      ['.', '3'],           // AB09
      ['\u11ae', '?'],      // AB10
      ['\u0020', '\u0020']  // SPCE
    ],
    'ko.romaja': [
      ['`', '~'],           // TLDE
      ['1', '!'],           // AE01
      ['2', '@'],           // AE02
      ['3', '#'],           // AE03
      ['4', '$'],           // AE04
      ['5', '%'],           // AE05
      ['6', '^'],           // AE06
      ['7', '&'],           // AE07
      ['8', '*'],           // AE08
      ['9', '('],           // AE09
      ['0', ')'],           // AE10
      ['-', '_'],           // AE11
      ['=', '+'],           // AE12
      ['\u314b', '\u110f'], // AD01
      ['\u116e', '\u116e'], // AD02
      ['\u1166', '\u1166'], // AD03
      ['\u3139', '\u1105'], // AD04
      ['\u314c', '\u1110'], // AD05
      ['\u1175', '\u1175'], // AD06
      ['\u116e', '\u116e'], // AD07
      ['\u1175', '\u1175'], // AD08
      ['\u1169', '\u1169'], // AD09
      ['\u314d', '\u1111'], // AD10
      ['[', '{'],           // AD11
      [']', '}'],           // AD12
      ['\\', '|'],          // BKSL
      ['\u1161', '\u1161'], // AC01
      ['\u3145', '\u1109'], // AC02
      ['\u3137', '\u1103'], // AC03
      ['\u314d', '\u1111'], // AC04
      ['\u3131', '\u1100'], // AC05
      ['\u314e', '\u1112'], // AC06
      ['\u3148', '\u110c'], // AC07
      ['\u314b', '\u110f'], // AC08
      ['\u3139', '\u1105'], // AC09
      [';', ':'],           // AC10
      ['\'', '"'],          // AC11
      ['\u3148', '\u110c'], // AB01
      ['\u11a8', '\u110c'], // AB02
      ['\u314a', '\u110e'], // AB03
      ['\u3142', '\u1107'], // AB04
      ['\u3142', '\u1107'], // AB05
      ['\u3134', '\u1102'], // AB06
      ['\u3141', '\u1106'], // AB07
      [',', '<'],           // AB08
      ['.', '>'],           // AB09
      ['/', '?'],           // AB10
      ['\u0020', '\u0020']  // SPCE
    ],
    'ko.ahn': [
      ['`', '~'],           // TLDE
      ['1', '!'],           // AE01
      ['2', '@'],           // AE02
      ['3', '#'],           // AE03
      ['4', '$'],           // AE04
      ['5', '%'],           // AE05
      ['6', '^'],           // AE06
      ['7', '&'],           // AE07
      ['8', '*'],           // AE08
      ['9', '('],           // AE09
      ['0', ')'],           // AE10
      ['-', '_'],           // AE11
      ['=', '+'],           // AE12
      ['\u1106', '\u1106'], // AD01
      ['\u1109', '\u317f'], // AD02
      ['\u1102', '\u1102'], // AD03
      ['\u1105', '\u1105'], // AD04
      ['\u1112', '\u3186'], // AD05
      ['\u1167', ';'],      // AD06
      ['\u1163', '\''],     // AD07
      ['\u1173', '/'],      // AD08
      ['\u116d', '['],      // AD09
      ['\u1172', ']'],      // AD10
      [',', '{'],           // AD11
      ['?', '}'],           // AD12
      ['\\', '|'],          // BKSL
      ['\u1107', '\u1107'], // AC01
      ['\u110c', '\u110c'], // AC02
      ['\u1103', '\u1103'], // AC03
      ['\u1100', '\u1100'], // AC04
      ['\u110b', '\u3181'], // AC05
      ['\u1165', '\u1165'], // AC06
      ['\u1161', '\u318d'], // AC07
      ['\u1175', '\u1175'], // AC08
      ['\u1169', '\u1169'], // AC09
      ['\u116e', ':'],      // AC10
      ['.', '"'],           // AC11
      ['\u11bd', '\u11bd'], // AB01
      ['\u11ae', '\u11ae'], // AB02
      ['\u11b8', '\u11b8'], // AB03
      ['\u11a8', '\u11a8'], // AB04
      ['\u11bc', '\u11f0'], // AB05
      ['\u11ba', '\u11eb'], // AB06
      ['\u11ab', '\u11ab'], // AB07
      ['\u11b7', '<'],      // AB08
      ['\u11af', '>'],      // AB09
      ['\u11c2', '\u11f9'], // AB10
      ['\u0020', '\u0020']  // SPCE
    ],
    'ko.set2y': [
      ['`', '~'],           // TLDE
      ['1', '!'],           // AE01
      ['2', '@'],           // AE02
      ['3', '#'],           // AE03
      ['4', '$'],           // AE04
      ['5', '%'],           // AE05
      ['6', '^'],           // AE06
      ['7', '&'],           // AE07
      ['8', '*'],           // AE08
      ['9', '('],           // AE09
      ['0', ')'],           // AE10
      ['-', '_'],           // AE11
      ['=', '+'],           // AE12
      ['\u3142', '\u3143'], // AD01
      ['\u3148', '\u3149'], // AD02
      ['\u3137', '\u3138'], // AD03
      ['\u3131', '\u3132'], // AD04
      ['\u3145', '\u3146'], // AD05
      ['\u116d', '\u116d'], // AD06
      ['\u1167', '\u1167'], // AD07
      ['\u1163', '\u1163'], // AD08
      ['\u1162', '\u1164'], // AD09
      ['\u1166', '\u1168'], // AD10
      ['[', '{'],           // AD11
      [']', '}'],           // AD12
      ['\\', '|'],          // BKSL
      ['\u3141', '\u1140'], // AC01
      ['\u3134', '\u115d'], // AC02
      ['\u3147', '\u114c'], // AC03
      ['\u3139', '\u111a'], // AC04
      ['\u314e', '\u1159'], // AC05
      ['\u1169', '\u1183'], // AC06
      ['\u1165', '\u1165'], // AC07
      ['\u1161', '\u119e'], // AC08
      ['\u1175', '\u1194'], // AC09
      [';', ':'],           // AC10
      ['\'', '"'],          // AC11
      ['\u314b', '\u113c'], // AB01
      ['\u314c', '\u113e'], // AB02
      ['\u314a', '\u114e'], // AB03
      ['\u314d', '\u1150'], // AB04
      ['\u1172', '\u1154'], // AB05
      ['\u116e', '\u1155'], // AB06
      ['\u1173', '\u1173'], // AB07
      [',', '<'],           // AB08
      ['.', '>'],           // AB09
      ['/', '?'],           // AB10
      ['\u0020', '\u0020']  // SPCE
    ],
    'ko.set3yet': [
      ['\u11f9', '\u11f0'], // TLDE
      ['\u11c2', '\u11bd'], // AE01
      ['\u11bb', '\u11eb'], // AE02
      ['\u11b8', '#'],      // AE03
      ['\u116d', '$'],      // AE04
      ['\u1172', '%'],      // AE05
      ['\u1163', '\u114c'], // AE06
      ['\u1168', '&'],      // AE07
      ['\u1174', '*'],      // AE08
      ['\u116e', '('],      // AE09
      ['\u110f', ')'],      // AE10
      ['-', '_'],           // AE11
      ['=', '+'],           // AE12
      ['\u11ba', '\u11c1'], // AD01
      ['\u11af', '\u11c0'], // AD02
      ['\u1167', '\u11bf'], // AD03
      ['\u1162', '\u1164'], // AD04
      ['\u1165', ';'],      // AD05
      ['\u1105', '<'],      // AD06
      ['\u1103', '·'],      // AD07
      ['\u1106', '\u1154'], // AD08
      ['\u110e', '\u1155'], // AD09
      ['\u1111', '>'],      // AD10
      ['[', '{'],           // AD11
      [']', '}'],           // AD12
      ['\\', '|'],          // BKSL
      ['\u11bc', '\u11ae'], // AC01
      ['\u11ab', '\u11ad'], // AC02
      ['\u1175', '\u11b0'], // AC03
      ['\u1161', '\u11a9'], // AC04
      ['\u1173', '\u119e'], // AC05
      ['\u1102', '\''],     // AC06
      ['\u110b', '\u114c'], // AC07
      ['\u1100', '\u114e'], // AC08
      ['\u110c', '\u1150'], // AC09
      ['\u1107', ':'],      // AC10
      ['\u1110', '"'],      // AC11
      ['\u11b7', '\u11be'], // AB01
      ['\u11a8', '\u11b9'], // AB02
      ['\u1166', '\u11b1'], // AB03
      ['\u1169', '\u11b6'], // AB04
      ['\u116e', '!'],      // AB05
      ['\u1109', '\u1140'], // AB06
      ['\u1112', '\u1159'], // AB07
      [',', '\u113c'],      // AB08
      ['.', '\u113e'],      // AB09
      ['\u1169', '?'],      // AB10
      ['\u0020', '\u0020']  // SPCE
    ],
    'ko.set32': [
      ['`', '\u203b'],      // TLDE
      ['1', '!'],           // AE01
      ['2', '@'],           // AE02
      ['3', '#'],           // AE03
      ['4', '$'],           // AE04
      ['5', '%'],           // AE05
      ['6', '^'],           // AE06
      ['7', '&'],           // AE07
      ['8', '*'],           // AE08
      ['9', '('],           // AE09
      ['0', ')'],           // AE10
      ['-', '_'],           // AE11
      ['=', '+'],           // AE12
      ['\u1107', '\u11b8'], // AD01
      ['\u110c', '\u11bd'], // AD02
      ['\u1103', '\u11ae'], // AD03
      ['\u1100', '\u11a8'], // AD04
      ['\u1109', '\u11ba'], // AD05
      ['\u116d', '\u116d'], // AD06
      ['\u1167', '\u1167'], // AD07
      ['\u1163', '\u1163'], // AD08
      ['\u1162', '\u1164'], // AD09
      ['\u1166', '\u1168'], // AD10
      ['[', '{'],           // AD11
      [']', '}'],           // AD12
      ['\\', '|'],          // BKSL
      ['\u1106', '\u11b7'], // AC01
      ['\u1102', '\u11ab'], // AC02
      ['\u110b', '\u11bc'], // AC03
      ['\u1105', '\u11af'], // AC04
      ['\u1112', '\u11c2'], // AC05
      ['\u1169', '\u1169'], // AC06
      ['\u1165', '\u1165'], // AC07
      ['\u1161', '\u1161'], // AC08
      ['\u1175', '\u1175'], // AC09
      [';', ':'],           // AC10
      ['\'', '"'],          // AC11
      ['\u110f', '\u11bf'], // AB01
      ['\u1110', '\u11c0'], // AB02
      ['\u110e', '\u11be'], // AB03
      ['\u1111', '\u11c1'], // AB04
      ['\u1172', '\u1172'], // AB05
      ['\u116e', '\u116e'], // AB06
      ['\u1173', '\u1173'], // AB07
      [',', '<'],           // AB08
      ['.', '>'],           // AB09
      ['/', '?'],           // AB10
      ['\u0020', '\u0020']  // SPCE
    ]
  };

  var viewIdPrefix = 'kokbd-k-';

  /**
   * Creates the Korean keyset data.
   *
   * @param {!Array.<!Array.<string>>} keyCharacters The key characters.
   * @return {!Object} The key data.
   */
  var createData = function(keyCharacters) {
    var data = util.createData(keyCharacters, viewIdPrefix, false, false);
    data[SpecNodeName.LAYOUT] = 'kokbd';
    // Adds the hangja key.
    var keyList = data[SpecNodeName.KEY_LIST];
    var spaceKeyIndex = -1;
    for (var i = keyList.length - 1; i >= 0; i--) {
      if (keyList[i]['spec'][SpecNodeName.ID] == 'Space') {
        spaceKeyIndex = i;
        break;
      }
    }
    if (spaceKeyIndex >= 0) {
      keyList.splice(spaceKeyIndex, 0,
          util.createIMESwitchKey('ControlRight', '한자', Css.JP_IME_SWITCH));
      var mapping = data[SpecNodeName.MAPPING];
      for (var i = spaceKeyIndex; i < keyList.length; i++) {
        mapping[keyList[i]['spec'][SpecNodeName.ID]] = 'kokbd-k-' + i;
      }
    }
    return data;
  };

  for (var id in characters) {
    var data = createData(characters[id]);
    data['id'] = id;
    google.ime.chrome.inputview.onConfigLoaded(data);
  }
})();
