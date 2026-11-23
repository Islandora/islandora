<?php

namespace Drupal\islandora_fcrepo\Controller;

use Drupal\image\Controller\ImageStyleDownloadController as CoreImageStyleDownloadController;
use Drupal\image\ImageStyleInterface;
use Symfony\Component\HttpFoundation\Request;

/**
 * Serves Fedora image styles through Drupal's access and token checks.
 */
class ImageStyleDownloadController extends CoreImageStyleDownloadController {

  /**
   * {@inheritdoc}
   */
  public function deliver(Request $request, $scheme, ImageStyleInterface $image_style, ?string $required_derivative_scheme = NULL) {
    return parent::deliver($request, $scheme, $image_style, $required_derivative_scheme ?? $scheme);
  }

}
