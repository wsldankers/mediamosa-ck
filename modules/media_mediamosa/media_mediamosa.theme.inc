<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * All theming functions.
 */

function theme_media_mediamosa_link($variables) {
  // Based on the asset_id and mediafile_id we will try to find the file in the
  // file_managed table.
  $uri = strtr('mediamosa://asset/@asset_id/%/%', array('@asset_id' => $variables['asset_id']));

  // Find in the file managed.
  $file_managed = db_select('file_managed', 'fm')
    ->fields('fm', array('fid', 'uri'))
    ->condition('uri', $uri, 'LIKE')
    ->range(0, 1)
    ->execute()->fetchAssoc();

  if (!$file_managed) {
    return t('Broken file');
  }

  // Get parts.
  $parts = MediaMosaStreamWrapper::mediamosa_parse_url($file_managed['uri']);

  // Return link to media.
  return l(check_plain($parts['filename']), 'file/' . $file_managed['fid']);
}

/**
 * Preview for Styles UI.
 */
function theme_media_mediamosa_preview_style($variables) {
  // TODO
  watchdog('media_mediamosa', t('Fixme: Style'), array(), WATCHDOG_WARNING);
  $variables['uri'] = 'FIXME: Theme';
  $variables['field_type'] = 'file';
  $variables['object'] = file_uri_to_object($variables['uri']);
  return theme('styles', $variables);
}

/**
 * Video player theme
 */
function theme_media_mediamosa_video($variables) {
    global $user;

    // To render, asset_id must be present.
    if (empty($variables['element']['#uri'])) {
      return;
    }
    
    $values = MediaMosaStreamWrapper::mediamosa_parse_url($variables['element']['#uri']);

    // Empty at start.
    $options = array();

    // What mediafile to play?
    $mediafile_id_to_play = empty($values['mediafile_id']) ? NULL : $values['mediafile_id'];

    // Take user_id of current watcher.
    $user_id = mediamosa_ck::session_user_id();

    // Determine admin user
    if (user_access('access administration pages')) {
      $options['is_app_admin'] = TRUE;
    }

    // Response.
    $options['response'] = 'object';

    // Not specified? Then use default transcode profile ID.
    if (empty($mediafile_id_to_play)) {
      $options['profile_id'] = MediaMosaCkConnectorWrapper::get_default_transcode_profile();
    }

    // Both not empty.
    if (!empty($variables['element']['#width']) && !empty($variables['element']['#height'])) {
      $options['width'] = $variables['element']['#width'];
      $options['height'] = $variables['element']['#height'];
      $options['autostart'] = $variables['element']['#autoplay'] ? 'TRUE' : 'FALSE';
    }

    try {
      $options['fatal'] = TRUE;
      $response = MediaMosaCkConnectorWrapper::get_play_link($values['asset_id'], $mediafile_id_to_play, $user_id, $options);
    }
    catch (Exception $e) {
      $response = theme('mediamosa_ck_views_player_image',
        array(
          'width' => $options['width'],
          'height' => $options['height'],
          'error_code' => $e->getCode(),
          'asset_id' => $values['asset_id'],
          'mediafile_id' => $mediafile_id_to_play,
        ));
    }
    return $response;
}